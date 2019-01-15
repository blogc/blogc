/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../common/utils.h"
#include "httpd-utils.h"


// mime types with index should be in the begin of the list. first NULL
// index aborts the lookup, for optimization
static const struct content_type_map {
    const char *mimetype;
    const char *extension;
    const char *index;
} content_types[] = {

    // with index
    {"text/html", "html", "index.html"},
    {"text/html", "htm", "index.htm"},
    {"text/html", "shtml", "index.shtml"},
    {"text/xml", "xml", "index.xml"},
    {"text/plain", "txt", "index.txt"},
    {"application/xhtml+xml", "xhtml", "index.xhtml"},

    // without index
    {"text/css", "css", NULL},
    {"image/gif", "gif", NULL},
    {"image/jpeg", "jpeg", NULL},
    {"image/jpeg", "jpg", NULL},
    {"application/javascript", "js", NULL},
    {"application/atom+xml", "atom", NULL},
    {"application/rss+xml", "rss", NULL},
    {"text/mathml", "mml", NULL},
    {"text/vnd.sun.j2me.app-descriptor", "jad", NULL},
    {"text/vnd.wap.wml", "wml", NULL},
    {"text/x-component", "htc", NULL},
    {"image/png", "png", NULL},
    {"image/tiff", "tif", NULL},
    {"image/tiff", "tiff", NULL},
    {"image/vnd.wap.wbmp", "wbmp", NULL},
    {"image/x-icon", "ico", NULL},
    {"image/x-jng", "jng", NULL},
    {"image/x-ms-bmp", "bmp", NULL},
    {"image/svg+xml", "svg", NULL},
    {"image/svg+xml", "svgz", NULL},
    {"image/webp", "webp", NULL},
    {"application/font-woff", "woff", NULL},
    {"application/java-archive", "jar", NULL},
    {"application/java-archive", "war", NULL},
    {"application/java-archive", "ear", NULL},
    {"application/json", "json", NULL},
    {"application/mac-binhex40", "hqx", NULL},
    {"application/msword", "doc", NULL},
    {"application/pdf", "pdf", NULL},
    {"application/postscript", "ps", NULL},
    {"application/postscript", "eps", NULL},
    {"application/postscript", "ai", NULL},
    {"application/rtf", "rtf", NULL},
    {"application/vnd.apple.mpegurl", "m3u8", NULL},
    {"application/vnd.ms-excel", "xls", NULL},
    {"application/vnd.ms-fontobject", "eot", NULL},
    {"application/vnd.ms-powerpoint", "ppt", NULL},
    {"application/vnd.wap.wmlc", "wmlc", NULL},
    {"application/vnd.google-earth.kml+xml", "kml", NULL},
    {"application/vnd.google-earth.kmz", "kmz", NULL},
    {"application/x-7z-compressed", "7z", NULL},
    {"application/x-cocoa", "cco", NULL},
    {"application/x-java-archive-diff", "jardiff", NULL},
    {"application/x-java-jnlp-file", "jnlp", NULL},
    {"application/x-makeself", "run", NULL},
    {"application/x-perl", "pl", NULL},
    {"application/x-perl", "pm", NULL},
    {"application/x-pilot", "prc", NULL},
    {"application/x-pilot", "pdb", NULL},
    {"application/x-rar-compressed", "rar", NULL},
    {"application/x-redhat-package-manager", "rpm", NULL},
    {"application/x-sea", "sea", NULL},
    {"application/x-shockwave-flash", "swf", NULL},
    {"application/x-stuffit", "sit", NULL},
    {"application/x-tcl", "tcl", NULL},
    {"application/x-tcl", "tk", NULL},
    {"application/x-x509-ca-cert", "der", NULL},
    {"application/x-x509-ca-cert", "pem", NULL},
    {"application/x-x509-ca-cert", "crt", NULL},
    {"application/x-xpinstall", "xpi", NULL},
    {"application/xspf+xml", "xspf", NULL},
    {"application/zip", "zip", NULL},
    {"application/octet-stream", "bin", NULL},
    {"application/octet-stream", "exe", NULL},
    {"application/octet-stream", "dll", NULL},
    {"application/octet-stream", "deb", NULL},
    {"application/octet-stream", "dmg", NULL},
    {"application/octet-stream", "iso", NULL},
    {"application/octet-stream", "img", NULL},
    {"application/octet-stream", "msi", NULL},
    {"application/octet-stream", "msp", NULL},
    {"application/octet-stream", "msm", NULL},
    {"application/vnd.openxmlformats-officedocument.wordprocessingml.document", "docx", NULL},
    {"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", "xlsx", NULL},
    {"application/vnd.openxmlformats-officedocument.presentationml.presentation", "pptx", NULL},
    {"audio/midi", "mid", NULL},
    {"audio/midi", "midi", NULL},
    {"audio/midi", "kar", NULL},
    {"audio/mpeg", "mp3", NULL},
    {"audio/ogg", "ogg", NULL},
    {"audio/x-m4a", "m4a", NULL},
    {"audio/x-realaudio", "ra", NULL},
    {"video/3gpp", "3gpp", NULL},
    {"video/3gpp", "3gp", NULL},
    {"video/mp2t", "ts", NULL},
    {"video/mp4", "mp4", NULL},
    {"video/mpeg", "mpeg", NULL},
    {"video/mpeg", "mpg", NULL},
    {"video/quicktime", "mov", NULL},
    {"video/webm", "webm", NULL},
    {"video/x-flv", "flv", NULL},
    {"video/x-m4v", "m4v", NULL},
    {"video/x-mng", "mng", NULL},
    {"video/x-ms-asf", "asx", NULL},
    {"video/x-ms-asf", "asf", NULL},
    {"video/x-ms-wmv", "wmv", NULL},
    {"video/x-msvideo", "avi", NULL},
    {NULL, NULL, NULL}
};


const char*
br_mime_guess_content_type(const char *filename)
{
    const char *extension = br_get_extension(filename);
    if (extension == NULL)
        goto default_type;
    for (size_t i = 0; content_types[i].extension != NULL; i++) {
        if (0 == strcmp(content_types[i].extension, extension)) {
            return content_types[i].mimetype;
        }
    }

default_type:
    return "application/octet-stream";
}


char*
br_mime_guess_index(const char *path)
{
    char *found = NULL;
    for (size_t i = 0; content_types[i].index != NULL; i++) {
        char *f = bc_strdup_printf("%s/%s", path, content_types[i].index);
        if (0 == access(f, F_OK)) {
            found = f;
            break;
        }
        free(f);
    }
    return found;
}
