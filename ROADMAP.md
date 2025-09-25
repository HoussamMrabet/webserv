# WebServ Location-Based Path Resolution Enhancement

## Project Overview
This roadmap details the enhancement of the WebServ HTTP server to properly handle location-based path resolution for GET requests, leveraging the existing infrastructure already present in the codebase.

## Current Architecture Analysis ✅

### Existing Infrastructure (ALREADY IMPLEMENTED)
1. **Request Class** - ✅ Complete location matching in `parser-request_line.cpp`
   - `getUri()` - Returns requested URI path
   - `getLocation()` - Returns matched location from config (✅ WORKING)
   - Location matching algorithm already functional (lines 75-95)

2. **ServerConf Class** - ✅ Full configuration support
   - `getRoot()` - Server-level document root
   - `getIndex()` - Server-level index files  
   - `getLocations()` - All location configurations

3. **LocationConf Class** - ✅ Complete location configuration
   - `getRoot()` - Location-specific root directory
   - `getIndex()` - Location-specific index files
   - `getName()` - Location path identifier

4. **CGI Implementation** - ✅ ALREADY HANDLES LOCATION-BASED RESOLUTION
   - File: `src/cgi.cpp` lines 74-98
   - ✅ Gets matched location from request
   - ✅ Uses location's root and index files
   - ✅ Handles directory requests by appending index file
   - ✅ Falls back to server root if needed

### Current Problem 🔍
**The `sendGetResponse()` function in Connection.cpp is NOT using the existing location-based path resolution logic.**

Current implementation:
```cpp
std::string requested_path = request.getUri();
std::string document_root = server.getRoot();  // Only uses server root!
std::string full_path = document_root + requested_path;
```

**This ignores the location matching that's already been done during request parsing!**

## Phase 1: Immediate Fix - Align GET with CGI Logic 🛠️

### Priority: HIGH - Quick Win
**Problem:** GET requests use basic server root, while CGI correctly uses location-based resolution

**Solution:** Apply the same logic from `cgi.cpp` to `sendGetResponse()`

### 1.1 Extract Location-Based Path Resolution Logic
**Status:** Ready to implement

```cpp
void Connection::sendGetResponse(Request &request, ServerConf &server) {
    Response response_obj;
    
    // Use the SAME logic as CGI implementation
    std::string requested_path = request.getUri();
    std::string matched_location = request.getLocation(); // Already calculated!
    
    // Get location configuration (same as CGI does)
    std::map<std::string, LocationConf> locations = server.getLocations();
    std::string document_root = server.getRoot(); // Default fallback
    std::vector<std::string> index_files = server.getIndex(); // Default fallback
    
    // If we have a matched location, use its configuration
    if (!matched_location.empty() && locations.find(matched_location) != locations.end()) {
        LocationConf location_conf = locations[matched_location];
        
        // Use location's root if available
        std::string location_root = location_conf.getRoot();
        if (!location_root.empty()) {
            document_root = location_root;
        }
               
        // Use location's index files if available
        std::vector<std::string> location_index = location_conf.getIndex();
        if (!location_index.empty()) {
            index_files = location_index;
        }
    }
    
    std::string full_path = document_root + requested_path;
    
    // Handle directory requests (same as CGI does)
    if (requested_path.empty() || requested_path.back() == '/') {
        if (!index_files.empty()) {
            full_path += index_files[0]; // Use first index file
        }
    }
    
    // Rest of implementation...
}
```

## Phase 2: Configuration Analysis �

### 2.1 Current Config Structure (from config.conf)
```
server {
    root /Users/cmasnaou/goinfre/web_main/www;  # Server-level root
    index index.html;                           # Server-level index
    
    location / {
        root /var/www/html;                     # Location-specific root
        index index.html;                       # Location-specific index
        allowed_methods GET POST DELETE;
    }
    
    location /upload {
        # Inherits server root if no location root specified
    }
}
```

### 2.2 Path Resolution Examples
```
Request: GET /
→ Matches location: "/"
→ Uses root: "/var/www/html" (from location)
→ Uses index: "index.html" (from location)  
→ Final path: "/var/www/html/index.html"

Request: GET /upload/
→ Matches location: "/upload"
→ Uses root: "/Users/cmasnaou/goinfre/web_main/www" (server fallback)
→ Uses index: "index.html" (server fallback)
→ Final path: "/Users/cmasnaou/goinfre/web_main/www/upload/index.html"

Request: GET /api/data.json
→ Matches location: "/" (longest match)
→ Uses root: "/var/www/html" (from location)
→ Final path: "/var/www/html/api/data.json"
```

## Phase 3: Response Class Enhancement 🔧

### 3.1 Current Response Capabilities ✅
**File:** `src/response/Response.cpp` (ALREADY IMPLEMENTED)

Available methods:
- ✅ `setStatus(int)` - HTTP status codes
- ✅ `setHeader(key, value)` - Add headers  
- ✅ `setBody(content)` - Set response body
- ✅ `buildResponse()` - Build final HTTP response
- ✅ `setBodyFromFile(path)` - Load file content (needs verification)

### 3.2 MIME Type Support ✅
**File:** `src/response/MimeTypes.cpp` (ALREADY EXISTS)

## Phase 4: Security & Error Handling 🔒

### 4.1 Existing Security Measures
**CGI already implements path validation:**
```cpp
bool CGI::validPath(){
    if (access(_scriptFileName.c_str(), F_OK) != 0) return false;
    if (access(_scriptFileName.c_str(), R_OK) != 0) return false;
    if (_scriptFileName.find("../") != std::string::npos) return false;
    return true;
}
```

**Apply same validation to GET requests.**

### 4.2 Error Page Configuration ✅
**Server already supports custom error pages:**
- `server.getErrorPages()` returns error page mappings
- Example: `404 → /error/404.html`

## Implementation Timeline �

### Week 1: Core Enhancement (High Priority)
- [ ] **Day 1-2:** Implement location-based path resolution in `sendGetResponse()`
- [ ] **Day 3:** Add directory index file handling
- [ ] **Day 4:** Implement security validation
- [ ] **Day 5:** Testing with current config

### Week 2: Integration & Polish
- [ ] **Day 1-2:** Error handling and custom error pages
- [ ] **Day 3:** MIME type integration  
- [ ] **Day 4-5:** Comprehensive testing

## Testing Strategy 🧪

### Test Current Configuration
```bash
# Test server root vs location root
curl -v http://localhost:8080/                    # Should use location "/" root
curl -v http://localhost:8080/upload/             # Should use server root
curl -v http://localhost:8080/nonexistent.html    # Should return 404

# Test index file resolution
curl -v http://localhost:8080/                    # Should serve index.html
curl -v http://localhost:8080/upload/             # Should serve upload/index.html
```

### Existing Test Files
```
www/
├── index.html           # Current server files
└── index2.html

/var/www/html/           # Location-specific files (from config)
├── index.html           # Different from server index
└── api/
    └── data.json
```

## Success Criteria ✅

### Immediate Goals (Week 1)
- [ ] GET requests use location-based root directories
- [ ] Directory requests serve location-specific index files
- [ ] Path resolution matches CGI behavior
- [ ] Security: Directory traversal prevention
- [ ] Error handling: 404, 403, 500 status codes

### Extended Goals (Week 2)  
- [ ] Custom error page serving
- [ ] Proper MIME type detection
- [ ] Integration testing with existing features
- [ ] Performance: Efficient file serving

## Code Quality Standards 📖

### C++98 Compliance ✅
- Already maintained in existing codebase
- No modern C++ features used
- Traditional for loops, explicit types
- Compatible with existing architecture

### Error Handling Pattern ✅
```cpp
// Follow existing pattern from cgi.cpp
try {
    // File operations
} catch (const std::exception& e) {
    response_obj.setStatus(500);
    response_obj.setBody("Internal Server Error");
}
```

## Next Steps 🚀

### Immediate Action Required
1. **Extract and adapt CGI path resolution logic** for GET requests
2. **Test with current configuration** to verify location matching
3. **Implement security validation** similar to CGI

### Future Enhancements
1. **Auto-index directory listing** (when configured)
2. **Range request support** for large files
3. **Conditional requests** (If-Modified-Since, ETag)

---

**Key Insight:** Most of the heavy lifting is already done. The location matching, configuration parsing, and path resolution logic exists and works correctly in CGI. We just need to apply the same logic to regular GET requests to make the behavior consistent across the entire server.

**Estimated Implementation Time:** 2-3 days for core functionality, 1 week for complete enhancement with testing.
