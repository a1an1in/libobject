/*
  Copyright (c) 2009 Dave Gamble

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libobject/core/utils/json/cjson.h>
#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/utils/dbg/debug.h>

void cjson_iterate(cjson_t *c);

/* Parse text to JSON, then render back to text, and print! */
void doit(char *text)
{
    char *out;
    cjson_t *json;

    json = cjson_parse(text);
    if (!json) {
        printf("Error before: [%s]\n", cjson_get_error_ptr());
    } else {
        out = cjson_print(json);
        cjson_delete(json);
        printf("%s\n", out);
        free(out);
    }
}

/* Read a file, parse, render back, etc. */
void dofile(char *filename)
{
    FILE *f;
    long len;
    char *data;

    /* open in read binary mode */
    f = fopen(filename,"rb");
    /* get the length */
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    data = (char*)malloc(len + 1);

    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);

    doit(data);
    free(data);
}

/* Used by some code below as an example datatype. */
struct record
{
    const char *precision;
    double lat;
    double lon;
    const char *address;
    const char *city;
    const char *state;
    const char *zip;
    const char *country;
};

/* Create a bunch of objects as demonstration. */
void create_objects(void)
{
    /* declare a few. */
    cjson_t *root;
    cjson_t *fmt;
    cjson_t *img;
    cjson_t *thm;
    cjson_t *fld;
    char *out;
    int i;

    /* Our "days of the week" array: */
    const char *strings[7] = {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
    };
    /* Our matrix: */
    int numbers[3][3] = {
        {0, -1, 0},
        {1, 0, 0},
        {0 ,0, 1}
    };
    /* Our "gallery" item: */
    int ids[4] = { 116, 943, 234, 38793 };
    /* Our array of "records": */
    struct record fields[2] = {
        {
            "zip",
            37.7668,
            -1.223959e+2,
            "",
            "SAN FRANCISCO",
            "CA",
            "94107",
            "US"
        }, {
            "zip",
            37.371991,
            -1.22026e+2,
            "",
            "SUNNYVALE",
            "CA",
            "94085",
            "US"
        }
    };
    volatile double zero = 0.0;

    /* Here we construct some JSON standards, from the JSON site. */

    /* Our "Video" datatype: */
    root = cjson_create_object();
    cjson_add_item_to_object(root, "name", cjson_create_string("Jack (\"Bee\") Nimble"));
    cjson_add_item_to_object(root, "format", fmt = cjson_create_object());
    cjson_add_string_to_object(fmt, "type", "rect");
    cjson_add_number_to_object(fmt, "width", 1920);
    cjson_add_number_to_object(fmt, "height", 1080);
    cjson_add_false_to_object (fmt, "interlace");
    cjson_add_number_to_object(fmt, "frame rate", 24);

    /* Print to text */
    out = cjson_print(root);
    /* Delete the cjson_t */
    cjson_delete(root);
    /* print it */
    printf("%s\n",out);
    /* release the string */
    free(out);

    /* Our "days of the week" array: */
    root = cjson_create_string_array(strings, 7);

    out = cjson_print(root);
    cjson_delete(root);
    printf("%s\n", out);
    free(out);

    /* Our matrix: */
    root = cjson_create_array();
    for (i = 0; i < 3; i++) {
        cjson_add_item_to_array(root, cjson_create_int_array(numbers[i], 3));
    }

    /* cjson_replace_item_in_array(root, 1, cjson_create_string("Replacement")); */

    out = cjson_print(root);
    cjson_delete(root);
    printf("%s\n", out);
    free(out);


    /* Our "gallery" item: */
    root = cjson_create_object();
    cjson_add_item_to_object(root, "Image", img = cjson_create_object());
    cjson_add_number_to_object(img, "Width", 800);
    cjson_add_number_to_object(img, "Height", 600);
    cjson_add_string_to_object(img, "Title", "View from 15th Floor");
    cjson_add_item_to_object(img, "Thumbnail", thm = cjson_create_object());
    cjson_add_string_to_object(thm, "Url", "http:/*www.example.com/image/481989943");
    cjson_add_number_to_object(thm, "Height", 125);
    cjson_add_string_to_object(thm, "Width", "100");
    cjson_add_item_to_object(img, "IDs", cjson_create_int_array(ids, 4));

    out = cjson_print(root);
    cjson_delete(root);
    printf("%s\n", out);
    free(out);

    /* Our array of "records": */

    root = cjson_create_array();
    for (i = 0; i < 2; i++) {
        cjson_add_item_to_array(root, fld = cjson_create_object());
        cjson_add_string_to_object(fld, "precision", fields[i].precision);
        cjson_add_number_to_object(fld, "Latitude", fields[i].lat);
        cjson_add_number_to_object(fld, "Longitude", fields[i].lon);
        cjson_add_string_to_object(fld, "Address", fields[i].address);
        cjson_add_string_to_object(fld, "City", fields[i].city);
        cjson_add_string_to_object(fld, "State", fields[i].state);
        cjson_add_string_to_object(fld, "Zip", fields[i].zip);
        cjson_add_string_to_object(fld, "Country", fields[i].country);
    }

    /* cjson_replace_item_in_object(cjson_get_array_item(root, 1), "City", cjson_create_int_array(ids, 4)); */

    out = cjson_print(root);
    cjson_delete(root);
    printf("%s\n", out);
    free(out);

    root = cjson_create_object();
    cjson_add_number_to_object(root, "number", 1.0 / zero);
    out = cjson_print(root);
    cjson_delete(root);
    printf("%s\n", out);
    free(out);
}

int test1(void)
{
    /* a bunch of json: */
    char text1[] =
        "{\n"
        "\"name\": \"Jack (\\\"Bee\\\") Nimble\", \n"
        "\"format\": {\"type\":       \"rect\", \n"
        "\"width\":      1920, \n"
        "\"height\":     1080, \n"
        "\"interlace\":  false,\"frame rate\": 24\n"
        "}\n"
        "}";
    char text2[] = "[\"Sunday\", \"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";
    char text3[] =
        "[\n"
        "    [0, -1, 0],\n"
        "    [1, 0, 0],\n"
        "    [0, 0, 1]\n"
        "\t]\n";
    char text4[] =
        "{\n"
        "\t\t\"Image\": {\n"
        "\t\t\t\"Width\":  800,\n"
        "\t\t\t\"Height\": 600,\n"
        "\t\t\t\"Title\":  \"View from 15th Floor\",\n"
        "\t\t\t\"Thumbnail\": {\n"
        "\t\t\t\t\"Url\":    \"http:/*www.example.com/image/481989943\",\n"
        "\t\t\t\t\"Height\": 125,\n"
        "\t\t\t\t\"Width\":  \"100\"\n"
        "\t\t\t},\n"
        "\t\t\t\"IDs\": [116, 943, 234, 38793]\n"
        "\t\t}\n"
        "\t}";
    char text5[] =
        "[\n"
        "\t {\n"
        "\t \"precision\": \"zip\",\n"
        "\t \"Latitude\":  37.7668,\n"
        "\t \"Longitude\": -122.3959,\n"
        "\t \"Address\":   \"\",\n"
        "\t \"City\":      \"SAN FRANCISCO\",\n"
        "\t \"State\":     \"CA\",\n"
        "\t \"Zip\":       \"94107\",\n"
        "\t \"Country\":   \"US\"\n"
        "\t },\n"
        "\t {\n"
        "\t \"precision\": \"zip\",\n"
        "\t \"Latitude\":  37.371991,\n"
        "\t \"Longitude\": -122.026020,\n"
        "\t \"Address\":   \"\",\n"
        "\t \"City\":      \"SUNNYVALE\",\n"
        "\t \"State\":     \"CA\",\n"
        "\t \"Zip\":       \"94085\",\n"
        "\t \"Country\":   \"US\"\n"
        "\t }\n"
        "\t ]";

    char text6[] =
        "<!DOCTYPE html>"
        "<html>\n"
        "<head>\n"
        "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
        "  <style type=\"text/css\">\n"
        "    html, body, iframe { margin: 0; padding: 0; height: 100%; }\n"
        "    iframe { display: block; width: 100%; border: none; }\n"
        "  </style>\n"
        "<title>Application Error</title>\n"
        "</head>\n"
        "<body>\n"
        "  <iframe src=\"//s3.amazonaws.com/heroku_pages/error.html\">\n"
        "    <p>Application Error</p>\n"
        "  </iframe>\n"
        "</body>\n"
        "</html>\n";

    /* Process each json textblock by parsing, then rebuilding: */
    doit(text1);
    doit(text2);
    doit(text3);
    doit(text4);
    doit(text5);
    doit(text6);

    /* Parse standard testfiles: */
    /* dofile("../../tests/test1"); */
    /* dofile("../../tests/test2"); */
    /* dofile("../../tests/test3"); */
    /* dofile("../../tests/test4"); */
    /* dofile("../../tests/test5"); */
    /* dofile("../../tests/test6"); */

    /* Now some samplecode for building objects concisely: */
    create_objects();

    return 0;
}

int test2(void)
{
    cjson_t *root;
    cjson_t *fmt;
    cjson_t *img;
    cjson_t *thm;
    cjson_t *fld, *c;
    char *out;
    int i;
    int ids[4] = { 116, 943, 234, 38793 };

    root = cjson_create_object();{
        cjson_add_item_to_object(root, "Image", img = cjson_create_object());{
            cjson_add_number_to_object(img, "Width", 800);
            cjson_add_number_to_object(img, "Height", 600);
            cjson_add_string_to_object(img, "Title", "View from 15th Floor");
            cjson_add_item_to_object(img, "Thumbnail", thm = cjson_create_object());{
                cjson_add_string_to_object(thm, "Url", "http:/*www.example.com/image/481989943");
                cjson_add_number_to_object(thm, "Height", 125);
                cjson_add_string_to_object(thm, "Width", "100");
            }
            cjson_add_item_to_object(img, "IDs", cjson_create_int_array(ids, 4));
        }
        cjson_add_string_to_object(root, "type", "rect");
        cjson_add_number_to_object(root, "width", 1920);
        cjson_add_number_to_object(root, "height", 1080);
        cjson_add_false_to_object (root, "interlace");
    }

    out = cjson_print(root);

    c = cjson_get_object_item(root, "Image");
    c = cjson_get_object_item(c, "Width");

    printf("width :%d\n", c->valueint);

    cjson_delete(root);
    printf("%s\n", out);
    free(out);

    return 0;
}

int test_json(void)
{
    /*
     *test1();
     */
    test2();
}

