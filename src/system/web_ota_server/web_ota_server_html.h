#pragma once

// HTML content for web server
const char INDEX_HTML[] = "<html><body><h1> SILIQS OTA By Web Server</h1>"
                          "<form method='POST' action='/update' enctype='multipart/form-data'>"
                          "<input type='file' name='update'>"
                          "<input type='submit' value='Update'></form></body></html>";