/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "../common/utils.h"
#include "lighttpd.h"

static const char conf[] =
    "server.document-root = \"%s\"\n"
    "server.bind = \"%s\"\n"
    "server.port = %hu\n"
    "server.errorlog = \"/dev/stderr\"\n"
    "\n"
    "mimetype.assign = (\n"
    "    \".html\" => \"text/html\",\n"
    "    \".htm\" => \"text/html\",\n"
    "    \".shtml\" => \"text/html\",\n"
    "    \".xml\" => \"text/xml\",\n"
    "    \".txt\" => \"text/plain\",\n"
    "    \".xhtml\" => \"application/xhtml+xml\",\n"
    "    \".css\" => \"text/css\",\n"
    "    \".gif\" => \"image/gif\",\n"
    "    \".jpeg\" => \"image/jpeg\",\n"
    "    \".jpg\" => \"image/jpeg\",\n"
    "    \".js\" => \"application/javascript\",\n"
    "    \".atom\" => \"application/atom+xml\",\n"
    "    \".rss\" => \"application/rss+xml\",\n"
    "    \".mml\" => \"text/mathml\",\n"
    "    \".jad\" => \"text/vnd.sun.j2me.app-descriptor\",\n"
    "    \".wml\" => \"text/vnd.wap.wml\",\n"
    "    \".htc\" => \"text/x-component\",\n"
    "    \".png\" => \"image/png\",\n"
    "    \".tif\" => \"image/tiff\",\n"
    "    \".tiff\" => \"image/tiff\",\n"
    "    \".wbmp\" => \"image/vnd.wap.wbmp\",\n"
    "    \".ico\" => \"image/x-icon\",\n"
    "    \".jng\" => \"image/x-jng\",\n"
    "    \".bmp\" => \"image/x-ms-bmp\",\n"
    "    \".svg\" => \"image/svg+xml\",\n"
    "    \".svgz\" => \"image/svg+xml\",\n"
    "    \".webp\" => \"image/webp\",\n"
    "    \".woff\" => \"application/font-woff\",\n"
    "    \".jar\" => \"application/java-archive\",\n"
    "    \".war\" => \"application/java-archive\",\n"
    "    \".ear\" => \"application/java-archive\",\n"
    "    \".json\" => \"application/json\",\n"
    "    \".hqx\" => \"application/mac-binhex40\",\n"
    "    \".doc\" => \"application/msword\",\n"
    "    \".pdf\" => \"application/pdf\",\n"
    "    \".ps\" => \"application/postscript\",\n"
    "    \".eps\" => \"application/postscript\",\n"
    "    \".ai\" => \"application/postscript\",\n"
    "    \".rtf\" => \"application/rtf\",\n"
    "    \".m3u8\" => \"application/vnd.apple.mpegurl\",\n"
    "    \".xls\" => \"application/vnd.ms-excel\",\n"
    "    \".eot\" => \"application/vnd.ms-fontobject\",\n"
    "    \".ppt\" => \"application/vnd.ms-powerpoint\",\n"
    "    \".wmlc\" => \"application/vnd.wap.wmlc\",\n"
    "    \".kml\" => \"application/vnd.google-earth.kml+xml\",\n"
    "    \".kmz\" => \"application/vnd.google-earth.kmz\",\n"
    "    \".7z\" => \"application/x-7z-compressed\",\n"
    "    \".cco\" => \"application/x-cocoa\",\n"
    ")\n"
    "\n"
    "index-file.names = (\n"
    "    \"index.html\",\n"
    "    \"index.htm\",\n"
    "    \"index.shtml\",\n"
    "    \"index.xml\",\n"
    "    \"index.txt\",\n"
    "    \"index.xhtml\",\n"
    ")\n";


char*
bm_lighttpd_deploy(const char *output_dir, const char *host, const char *port)
{
    if (output_dir == NULL)
        return NULL;

    unsigned short port_int = 8080;
    if (port  != NULL) {
        char *endptr;
        port_int = strtoul(port, &endptr, 10);
        if (*port != '\0' && *endptr != '\0') {
            fprintf(stderr, "blogc-make: error: Failed to parse lighttpd port "
                "value: %s\n", port);
            return NULL;
        }
    }

    // this is not really portable
    char fname[] = "/tmp/blogc-make_XXXXXX";
    int fd;
    if (-1 == (fd = mkstemp(fname))) {
        fprintf(stderr, "blogc-make: error: Failed to create temporary "
            "lighttpd config: %s", strerror(errno));
        return NULL;
    }

    const char *host_str = host == NULL ? "127.0.0.1": host;
    char *content = bc_strdup_printf(conf, output_dir, host_str, port_int);

    if (-1 == write(fd, content, strlen(content))) {
        fprintf(stderr, "blogc-make: error: Failed to write to temporary "
            "lighttpd config: %s", strerror(errno));
        free(content);
        close(fd);
        unlink(fname);
        return NULL;
    }

    free(content);
    close(fd);

    fprintf(stderr, " * Running on http://%s:%hu/\n\n", host_str, port_int);

    return bc_strdup(fname);
}


void
bm_lighttpd_destroy(const char *fname)
{
    unlink(fname);
}
