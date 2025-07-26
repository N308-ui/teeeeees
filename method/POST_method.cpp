#include "../network/server.hpp"
#include "../header/class_header.hpp"
#include "method_headher.hpp"
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

void Server::handlePostRequest(int client_fd, const RequestParser& parser) {
    // Find matching location block
    int location_index = mathch_location(parser.getUri());
    if (location_index == -1) {
        sendErrorResponse(client_fd, 404, "Not Found");
        return;
    }

    const LocationBlock& location = data.getLocations()[location_index];

    // Check if POST is allowed for this location
    if (std::find(location.getMethods().begin(), location.getMethods().end(), "POST") == location.getMethods().end()) {
        sendErrorResponse(client_fd, 405, "Method Not Allowed");
        return;
    }

    // Check if upload is supported (POST allowed + upload_dir set)
    bool supports_upload = !location.getUploadDir().empty();
    
    if (supports_upload) {
        // Handle file upload
        handleFileUpload(client_fd, parser, location);
    } else {
        // Regular POST request processing
        std::string response_body = "POST request received\n";
        response_body += "URI: " + parser.getUri() + "\n";
        response_body += "Content-Type: " + parser.getHeader("content-type") + "\n";
        response_body += "Body length: " + to_string(parser.getBody().length()) + "\n";
        
        responses[client_fd] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n" + response_body;
    }
}

void Server::handleFileUpload(int client_fd, const RequestParser& parser, const LocationBlock& location) {
    // Check if content-type is multipart/form-data
    std::string content_type = parser.getHeader("content-type");
    if (content_type.find("multipart/form-data") == std::string::npos) {
        sendErrorResponse(client_fd, 400, "Bad Request: Expected multipart/form-data");
        return;
    }

    // Extract boundary from content-type
    size_t boundary_pos = content_type.find("boundary=");
    if (boundary_pos == std::string::npos) {
        sendErrorResponse(client_fd, 400, "Bad Request: Missing boundary in Content-Type");
        return;
    }
    
    std::string boundary = content_type.substr(boundary_pos + 9); // 9 is length of "boundary="
    if (boundary.empty()) {
        sendErrorResponse(client_fd, 400, "Bad Request: Empty boundary");
        return;
    }

    // Process multipart data
    const std::string& body = parser.getBody();
    std::string full_boundary = "--" + boundary;
    size_t pos = 0;
    bool file_saved = false;

    while (pos < body.length()) {
        // Find boundary
        size_t boundary_start = body.find(full_boundary, pos);
        if (boundary_start == std::string::npos) break;
        
        // Find headers section
        size_t headers_start = boundary_start + full_boundary.length();
        if (headers_start >= body.length()) break;
        
        // Find end of headers
        size_t headers_end = body.find("\r\n\r\n", headers_start);
        if (headers_end == std::string::npos) break;
        
        // Parse headers
        std::string headers_part = body.substr(headers_start, headers_end - headers_start);
        std::istringstream headers_stream(headers_part);
        std::string header_line;
        std::string filename;
        std::string name;
        
        while (std::getline(headers_stream, header_line)) {
            header_line = trimWhitespace(header_line);
            if (header_line.empty()) continue;
            
            // Check for Content-Disposition
            if (header_line.find("Content-Disposition:") == 0) {
                size_t name_pos = header_line.find("name=\"");
                if (name_pos != std::string::npos) {
                    size_t name_end = header_line.find("\"", name_pos + 6);
                    if (name_end != std::string::npos) {
                        name = header_line.substr(name_pos + 6, name_end - (name_pos + 6));
                    }
                }
                
                size_t filename_pos = header_line.find("filename=\"");
                if (filename_pos != std::string::npos) {
                    size_t filename_end = header_line.find("\"", filename_pos + 10);
                    if (filename_end != std::string::npos) {
                        filename = header_line.substr(filename_pos + 10, filename_end - (filename_pos + 10));
                    }
                }
            }
        }
        
        // Get file data
        size_t data_start = headers_end + 4;
        size_t data_end = body.find(full_boundary, data_start);
        if (data_end == std::string::npos) {
            data_end = body.length() - 2; // Last boundary has -- at the end
        }
        
        if (!filename.empty()) {
            // Save file
            std::string upload_dir = location.getUploadDir();
            
            // Ensure upload directory exists
            struct stat st;
            if (stat(upload_dir.c_str(), &st) != 0) {
                if (mkdir(upload_dir.c_str(), 0755) != 0) {
                    sendErrorResponse(client_fd, 500, "Internal Server Error: Could not create upload directory");
                    return;
                }
            }
            
            // Create full path
            std::string file_path = upload_dir + "/" + filename;
            
            // Write file
            std::ofstream out_file(file_path.c_str(), std::ios::binary);
            if (!out_file) {
                sendErrorResponse(client_fd, 500, "Internal Server Error: Could not create file");
                return;
            }
            
            out_file.write(body.data() + data_start, data_end - data_start - 2); // -2 for \r\n
            out_file.close();
            file_saved = true;
        }
        
        pos = data_end;
    }
    
    if (file_saved) {
        responses[client_fd] = "HTTP/1.1 201 Created\r\nContent-Type: text/plain\r\n\r\nFile uploaded successfully";
    } else {
        sendErrorResponse(client_fd, 400, "Bad Request: No file uploaded");
    }
}

std::string Server::trimWhitespace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}