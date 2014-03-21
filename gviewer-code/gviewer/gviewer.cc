/***************************************************************************
 *   Copyright (C) 2012 by Gabriel A Hernandez C                           *
 *   gabriel.hernandez@gmx.ch                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY!!; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301 USA.                                            *
 ***************************************************************************/

/*
    TODO : Clean unsued heathers....
*/
#include <stdio.h>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sstream>
// #include <ctype.h>
// #include <sys/types.h>
#include <regex.h>
#include <getopt.h>
// #include <unistd.h>
#include <math.h>
#include <sys/ioctl.h>
#include <Error.h>
#include <UnicodeMap.h>
#include <GString.h>
#include "gtypes.h"
#include "parseargs.h"
#include <PDFDoc.h>
#include <TextOutputDev.h>
#include <GlobalParams.h>
#include <Page.h>

#include "config.h"
#include <Magick++.h>

#define BUFFER_SIZE 1024

using namespace Magick;
using namespace std;

static char *filename_color;
static char *pagenum_color;
static char *highlight_color;
static char *seperator_color;

static char *PACKAGE;
static char *VERSION;
const static char *gViewerVersion = "0.01";

// DEBUGING strings...
GBool debugging = gFalse;

/* set this to 1 if any match was found. Used for the exit status */
int found_something = 0;

/* default options */

int ignore_case = 0;
int color = 1;
/* characters of context to put around each match.
 * -1 means: print whole line
 * -2 means: try to fit context into terminal */
int context = -2;
int line_width = 80;
int print_filename = -1;
int print_pagenum = 0;
int countfounds = 0;
int quiet = 0;
int antialiasing = 4;


#define HELP_OPTION 1
#define COLOR_OPTION 2

// assuming that gs is located here... 
#define GS "/usr/bin/gs"

static double dpi = 150;
static int firstPage = 1;
static int lastPage = 0;
static int scaleThumbsFactor = 14;
static GBool ignoreCase = gFalse;
static GBool printHelp = gFalse;
static GBool printVersion = gFalse;
static GBool errQuiet = gFalse;
static GBool onlyThumbnails = gFalse;
// if no output filename provided, the PDF path and filename will be taken
// to generate the pages... 
static char outputFilename[256] = "";

// ARGUMENTS
static ArgDesc argDesc[] = {
    {"-f", argInt, &firstPage, 0, "First page to convert"},
    {"-l", argInt, &lastPage, 0, "Last page to convert"},
    {"-i", argFlag, &ignoreCase, 0, "Ignore case search"},
    {"-q", argFlag, &errQuiet, 0, "Quiet, don't print any results nor errors"},
    {"-h", argFlag, &printHelp, 0, "Print usage information"},
    {"-o", argString, outputFilename, sizeof(outputFilename), "Output path and filename of produced pages: <./foo_%d.jpg>"},
    {"-thumbs", argFlag, &onlyThumbnails, sizeof(onlyThumbnails), "Generate thumbnails only"},
    {"-help", argFlag, &printHelp, 0, "Print usage information"},
    {"--help", argFlag, &printHelp, 0, "Print usage information"},
    {"-v", argFlag, &printVersion, 0, "Print version"},
    {"-version", argFlag, &printVersion, 0, "Print version"},
    {"--version", argFlag, &printVersion, 0, "Print version"},
    {"--debug", argFlag, &debugging, sizeof(debugging), "Outputs debuging text to the screen"},
    {NULL}
};


/* --------------------
// NOTE: FUNCTION Declarations
---------------------- */
// These two functions are part of the example...
// http://cboard.cprogramming.com/c-programming/65581-dynamic-string-array.html
// add a string to a list of strings...
char **add_string(char **list, char *add, size_t index);

// middle man... calls add string function...
void foundWordsList(char *list, char *add, size_t index);

// Here starts my implementation
// Instead of printing agregate search to found terms list
void addFoundToList(char ***wordsList, size_t *elementsInList, char *string, int pos, int matchend);

// This is a duplicate of PUTSN putsn
void putFoundItem(char ***wordsList, size_t *elementsInList, char *string, int from, int to);

bool isTermInList(char **wordsList, size_t indexList, GString *foundItem);

//  get and store the terms coordinates... \
    receives the PDF document and the term to be found
int get_terms_coordinates(PDFDoc *doc, const char *text);

void highlight_found_text(PDFDoc *, size_t, GString *);

// HELPER functions...
// helper to convert printing points to screen pixels
double pointToPixels( double );

// create the images...
GBool gs_create_pages(PDFDoc, GString PDFFilename, int);

// extracts the filename from a provided filename with path
GString *getOutputFilename(GString *PDFFilename);

// extracts the filename from the provided PDF filename to be used as output 
GString *getPathFromPDFFilename(GString *, GString *);

// we will open only the pages that need highlighting
void get_pages_to_highlight(vector<int> *);

// get the PDFFilename or outputFilename argument
GString *get_output_filename(GString *);

// Generate thumbnails for not highlighted pages...
void get_page_thumbnail(int, GString *);

// struct option long_options[] =
// {
//     {"ignore-case", 0, 0, 'i'},
//     {"page-number", 0, 0, 'n'},
//     {"with-filename", 0, 0, 'H'},
//     {"no-filename", 0, 0, 'h'},
//     {"countfounds", 0, 0, 'c'},
//     {"color", 1, 0, COLOR_OPTION},
//     {"context", 1, 0, 'C'},
//     {"help", 0, 0, HELP_OPTION},
//     {"version", 0, 0, 'V'},
//     {"quiet", 0, 0, 'q'},
//     {0, 0, 0, 0}
// };

// --------------------------
// NOTE: TYPES definitions
// --------------------------

// The stream....
struct stream {
    int bufsize;
    char *buf;
    int charpos;
};

struct stream* reset_stream(struct stream *s) {
    s->bufsize = BUFFER_SIZE;
    free(s->buf);
    s->buf = (char*) malloc(BUFFER_SIZE);
    s->charpos = 0;

    return s;
}

struct stream* make_stream() {
    struct stream *s = (struct stream*) malloc(sizeof(struct stream));

    s->buf = NULL;

    return reset_stream(s);
}

void maybe_update_buffer(struct stream *s, int len) {
    if (s->charpos + len >= s->bufsize) {
        /* resize buffer */
        do
            s->bufsize = s->bufsize * 2;
        while (s->charpos + len >= s->bufsize);

        s->buf = (char*) realloc(s->buf, s->bufsize);
    }
}

void write_to_stream(void *s, char *text, int length)
{
    struct stream *stream = (struct stream*) s;

    maybe_update_buffer(stream, length);
    strncpy(stream->buf+stream->charpos,
            text, length);
    stream->charpos += length;
}


// Starting Rectangle definition; this will allocate the positions found to form rectagles for highlighting
struct gviewer_rectangle {
        double  xMin;
        double  yMin;
        double  xMax;
        double  yMax;
        int     page;
        GString    *term;
    };
    
struct gviewer_rectangle *create_rectangle_highlight(double xMin, double yMin, double xMax, double yMax, unsigned int page, GString *term) {

    // get memory
    struct gviewer_rectangle *g_rectangle = (struct gviewer_rectangle*) malloc(sizeof(struct gviewer_rectangle));

    assert(g_rectangle != NULL);
    
    g_rectangle->xMin = xMin;
    g_rectangle->yMin = yMin;
    g_rectangle->xMax = xMax;
    g_rectangle->yMax = yMax;
    g_rectangle->page = page;
    g_rectangle->term = term;
    
    return g_rectangle;
}

void g_rectangle_destroy(struct gviewer_rectangle *g_rectangle) {
    assert(g_rectangle != NULL);
    free(g_rectangle);
}

void print_g_rectangle(struct gviewer_rectangle *g_rectangle) {
    printf("Page:\t%d\t", g_rectangle->page);
    printf("Term:\t%s\t", g_rectangle->term->getCString());
    printf("xMin:\t%f\t", g_rectangle->xMin);
    printf("yMin:\t%f\t", g_rectangle->yMin);
    printf("xMax:\t%f\t", g_rectangle->xMax);
    printf("yMax:\t%f\n", g_rectangle->yMax);
}

// get coordinates of highlighting box from struc
double xMinRec(struct gviewer_rectangle *g_rectangle) {
    return g_rectangle->xMin;
}

double yMinRec(struct gviewer_rectangle *g_rectangle) {
    return g_rectangle->yMin;
}

double xMaxRec(struct gviewer_rectangle *g_rectangle) {
    return g_rectangle->xMax;
}

double yMaxRec(struct gviewer_rectangle *g_rectangle) {
    return g_rectangle->yMax;
}

int pageRec(struct gviewer_rectangle *g_rectangle) {
    return g_rectangle->page;
}

GString *termRec(struct gviewer_rectangle *g_rectangle) {
    return g_rectangle->term;
}
// end rectangle definition

// NOTE: Vector definitions
vector<gviewer_rectangle> rectangleHighlights;


bool isTermInList(char **wordsList, size_t indexList, GString *foundItem) {
    bool found = false;
    size_t i = 0;
    
    while(i < indexList)
        {   
            // if (debugging) {
            //                 printf("////-----  INDEXLIST [ %u ]: \n", i);
            //                 // storedTerm = (*wordsList[i]);
            //                 printf("passing...%s\n" , foundItem->getCString());
            //             }
            if( debugging ) {
                fprintf(stderr, "Comparando: found:[ %s ] con stored:[ %s ] === [ %d ] \n", foundItem->getCString(), (wordsList[i]),  ( foundItem->cmp( (wordsList[i]) ) == 0) );
            }
            
            if( foundItem->cmp( (wordsList[i]) ) == 0 ) {
                found = true;
                // if (debugging)
                //                     printf("..................[ Stored | SKIPIING ]...................\n");
                break;
            }
            i++;
        }
        return found;
}

//------------------- NOTE:  Implemetation START here

// This is a duplicate of PUTSN putsn
void putFoundItem(char ***wordsList, size_t *elementsInList, char *string, int from, int to) {

    // current term to compare in list
    GString *foundItem;
    
    // assume that the term is not in the list
    GBool isInList = gFalse;
    
    // sut current term space
    foundItem = new GString();
    
    // the term in the loop from the list to campare to
    char *storedTerm = NULL;
    
    // indexes
    size_t tmp, indexList, i;
    
    // get the number of items in the list
    tmp = *elementsInList;
    indexList = *elementsInList;
    i = 0;

    // if (debugging)
    //     printf("tmp: %u | indexList: %u | elementsInList: %u\n", tmp, indexList, *elementsInList );
    
    // extract term from the stream
    for (; from < to; from++) {
        foundItem->append(string[from]);
    }
    foundItem->append("\0");

    // DELETE ME: conver to lowercase; not anymore needed... 
    // if(ignoreCase)
    //     foundItem->lowerCase();

    // put first element in the list
    if( indexList == 0) {
        (*wordsList) = add_string( (**&wordsList), foundItem->getCString(), indexList);
        tmp++;
        isInList = gTrue;
            if( debugging ) {
                printf("almacenando.....[ %s ]....... | en [ %d ]\t", foundItem->getCString(), indexList);
                printf("-------------elementsInList: %d\n", tmp);
            }
    } else {
        if( isTermInList(*wordsList, indexList, foundItem) ) {
            isInList = gTrue;
        }
    }
    
    // store term if it does not exist in teh list
    if( !isInList ) {
        (*wordsList) = add_string( (**&wordsList), foundItem->getCString(), indexList);
        tmp++;
        isInList = gTrue;
            if(debugging) {
                printf("almacenando.....[ %s ]....... | en [ %d ]\t", foundItem->getCString(), indexList);
                // printf("Term in list [ %s ] en posicion [ %d ]\n", (*&wordsList[indexList]), indexList);
                printf("-------------elementsInList: %d\n", tmp);
            }
    }

    *elementsInList = tmp;
    free (foundItem);
    // free(isTermInList);
}

// Instead of printing agregate search to found terms list
void addFoundToList(char ***wordsList, size_t *elementsInList, char *string, int pos, int matchend) {
    putFoundItem((*&wordsList), *&elementsInList, string, pos, matchend);    
}

//----------------- NOTE: Implementation END here...

//-------- NOTE: Dynamic array -- found terms... start
// NOTE: ---------- add a string to a list---- MINE
// char **add_string(char **list, char *add, size_t index) {
//     char **newList;
//     int count;
//     
//     // assign memory
//     newList = (char **) malloc(sizeof(char *) * (index + 1 ));
//     
//     if(index > 0)
//     {
//         for(count = 0; count < index; count++)
//         {
//             newList[count] = list[count];
//         }
//     }
//     newList[index] = (char *) malloc(strlen(add) + 1);
//     strcpy( newList[index], add);
//     
//     free(list);
//     return newList;
// }


char **add_string(char **list, char *add, size_t size)
{
    char **newList;
    int count;
 
    newList = (char **) malloc( sizeof( char* ) * ( size + 1 ) );
 
    if( size > 0 ) {
        for( count = 0; count < size; count++ ) {
            newList[count] = list[count];
        }
    }
     
    newList[size] = (char *) malloc( strlen( add ) + 1 );
    strcpy( newList[size], add );
 
    free( list );
    return newList;
}



void foundWordsList(char ***list, char *add, size_t index) {
    (*list) = add_string((*list), add, index);
}


/*
    NOTE: INDOC in document sear
*/
int search_in_document(PDFDoc *doc, regex_t *needle, char ***wordsList, size_t *elementsInList) {
    
    GString *filename = doc->getFileName();
    struct stream *stream = make_stream();
    regmatch_t match[] = {{0, 0}};
    int count_matches = 0;
    int length = 0;
    int oldcontext;
    int len = 0;
    Unicode *u;
    int numTotalPages;
    
    GBool startAtTop, stopAtBottom, startAtLast, stopAtLast, backward;

    double *xMin, *yMin, *xMax, *yMax;
    
    startAtTop = gTrue;
    stopAtBottom = gTrue;
    startAtLast = gFalse;
    stopAtLast = gFalse;
    backward = gTrue;
    GBool foundText;
    
    xMin = yMin = xMax = yMax = 0;
    
    /*
       TODO : This arguments should be configurable...
    */
    TextOutputDev *text_out = new TextOutputDev(write_to_stream, stream, gFalse, gTrue);
        
    if (!text_out->isOk()) {
        if (!errQuiet) {
            fprintf(stderr, "gViewer ERROR: Could not search %s\n", filename->getCString());
        }
        goto clean;
    }
    
    // Do all the pages if there is not a range or single page provided
    if ( lastPage == 0 )
        numTotalPages = doc->getNumPages();
    else
        numTotalPages = lastPage;
    
    if(debugging)
        fprintf(stderr, "firstPage: %d | numTotalPages: %d\n", firstPage, numTotalPages);

    // CONTADOR
    for (int i = firstPage; i <= numTotalPages; ++i) {
        doc->displayPage(text_out, i,
                72.0, 72.0, 0,
                gFalse, gTrue, gFalse);
                
        stream->buf[stream->charpos] = '\0';
        int index = 0;
        // NOTE: basic implementation to store terms in list
        
        while (!regexec(needle, stream->buf+index, 1, match, REG_EXTENDED)) {
            // store term in list of found terms in document
            addFoundToList((*&wordsList), *&elementsInList, stream->buf, index + match[0].rm_so, index + match[0].rm_eo);
            found_something = 1;
            index += match[0].rm_so + 1;
            count_matches++;
        }

        reset_stream(stream);
    } // for... 
    
clean:
    free(stream->buf);
    free(stream);
    delete text_out;

    return count_matches;
}

// returns number of found matches in the entire document...
int get_terms_coordinates(PDFDoc *doc, const char *text){
    double xMin, yMin, xMax, yMax;
    Unicode *u;
    int u_len;
    int len;
    double height;
    TextOutputDev *textOut;
    
    GBool startAtTop = gFalse;
    GBool stopAtBottom = gTrue;
    GBool startAtLast = gFalse;
    GBool stopAtLast = gFalse;
    GBool caseSensitive;
    GBool backwards = gFalse;
    TextPage *text_page;
    GString *foundText;
    int counter, totalPages, totalFound;

    // set case sensitive base on arguments...
    if( ignoreCase )
        caseSensitive = ignoreCase;
        
    // convert text to unicode
    len = strlen(text);
    u = (Unicode *) gmallocn(len, sizeof(Unicode));
        for (int i = 0; i < len; ++i) {
            u[i] = (Unicode)(text[i] & 0xff);
        }

    u_len = sizeof(u);
    
    // check if we have provided a range... 
    if( lastPage == 0 )
        totalPages = doc->getNumPages();
    else 
        totalPages = lastPage;

    if(debugging)
        fprintf(stderr, "Starting search engine...................\n");
    
    // go through all the pages...
    totalFound = 0;
    
    //  FIXME: here you should do only the page range provided... \
        perhaps get firstPage and lastPage...
    for( int i = firstPage; i <= totalPages; ++i )
    {
        xMin = 0;
        yMin = 0;
        counter = 0;
                
        // get the page content
        textOut = new TextOutputDev(NULL, gFalse, gFalse, gTrue); // set append to true to preserv the content of the page...
        // display current page...
        // doc->displayPage(textOut, pg, dpi, dpi, rotate, usemediabox, crop, links); 
        doc->displayPage(textOut, i, 72.0, 72.0, 0, gFalse, gTrue, gFalse);
        
        // perform search!..
         while( textOut->findText(u, len,
            startAtTop, stopAtBottom,
            startAtLast, stopAtLast,
            caseSensitive, backwards,
            &xMin, &yMin, &xMax, &yMax)) {
            counter++;
        
            // get me the page number
            if(debugging)
                fprintf(stderr, "Page:\t%d", i);

            foundText = NULL;
            foundText = textOut->getText(xMin, yMin, xMax, yMax);
                   
            if (debugging) 
                fprintf(stderr, "\tFound: %s", foundText->getCString());
                                      
            // TODO : add configuration variable here.. Print coordinates or not...
            if( debugging ) {
                fprintf(stderr, "\txMin: \t%f", pointToPixels(xMin) );
                fprintf(stderr, "\tyMin: \t%f", pointToPixels(yMin) );
                fprintf(stderr, "\txMax: \t%f", pointToPixels(xMax) );
                fprintf(stderr, "\tyMax: \t%f\n", pointToPixels(yMax) );
            }
                  
            // WORKING: store coordinates in struct...
            rectangleHighlights.push_back( *create_rectangle_highlight( pointToPixels(xMin), pointToPixels(yMin), pointToPixels(xMax), pointToPixels(yMax), i, foundText));
        } // while

        totalFound+=counter;
        
        if (debugging)
            fprintf(stderr, "Items found: \t%d\n---------------------\n", counter);
    } //for

    free(u);
    delete textOut;
    if(debugging)
        fprintf(stderr, "Total times found per item : \t%d\n---------------------\n", totalFound);
    return totalFound;
}

// NOTE:  Points to Pixels...
// formula: (points * 150) / 72
double pointToPixels( double puntos ) {
    double depth = 72;
    return ( puntos * dpi ) / depth;
}


// NOTE:  Highlight found text...: Drawing a rectangle...
void highlight_found_text(PDFDoc *doc, size_t page, GString *PDFFilename) {
    
    GString *tmpOutputFilename;
    GString *thumbsFilename;
    
    // will collect all the rectangles to highlight
    list<Drawable> objects_to_draw;
    
    // compose the filename of the page to highlight
    tmpOutputFilename = get_output_filename(PDFFilename);
    tmpOutputFilename->append("_");
    tmpOutputFilename->append(tmpOutputFilename->fromInt(page));
    tmpOutputFilename->append(".jpg");
     try {
    // Create base image with PDF in the background...
    Image ImagePage;
    ImagePage.read(tmpOutputFilename->getCString());
    // highlights...
    Image HighlightBoxes(ImagePage.size(),Color(0,0,0,MaxRGB));
    
    
    // set some Image defaults...
    // image.fillColor( Color("alpha") );
        
    // loop the struct for the coordinates of this particular page...
    for(size_t i = 0; i < rectangleHighlights.size(); ++i) {
        if(pageRec(&rectangleHighlights[i]) == page) {
            // output the highlight locations... this for possibly frontend implementation
            if(!errQuiet)
                print_g_rectangle(&rectangleHighlights[i]);
            // objects_to_draw.push_back(DrawableFillOpacity(500));
            objects_to_draw.push_back(DrawableStrokeColor("#FFEB00"));
            // objects_to_draw.push_back(DrawableStrokeOpacity(500));
            objects_to_draw.push_back(DrawableStrokeWidth(2));
            objects_to_draw.push_back(DrawableFillColor(Color("#FFFF00")));
            // store coordinates...
            objects_to_draw.push_back(DrawableRectangle( xMinRec(&rectangleHighlights[i]), yMinRec(&rectangleHighlights[i]), xMaxRec(&rectangleHighlights[i]), yMaxRec(&rectangleHighlights[i]) ) );
        }
    }
    
    HighlightBoxes.draw(objects_to_draw);
    ImagePage.composite( HighlightBoxes, 0, 0, MultiplyCompositeOp );
    ImagePage.transparent("White");
    ImagePage.matte( false );
    
    // REGRESA...
    // Create thubnails...
    thumbsFilename = getPathFromPDFFilename(tmpOutputFilename, new GString(".jpg"));
    thumbsFilename->append("_thumb.jpg");
    Image PageThumbnails(ImagePage);
    PageThumbnails.resolutionUnits(PixelsPerInchResolution); 
    PageThumbnails.density(Geometry(72,72));
    PageThumbnails.scale(Geometry((PageThumbnails.columns() / scaleThumbsFactor), (PageThumbnails.rows() / scaleThumbsFactor)));
    // write thumbnails...
    PageThumbnails.write(thumbsFilename->getCString());
    // white highlighted pages...
    ImagePage.write( tmpOutputFilename->getCString() );
    
    // ImagePage.display();
    
    }
    catch (Exception &error_) {
        cout << error_.what() << endl; 
    }
    /*
        TODO: Create thumbnail of the current image before saving big image...
    */

    // ImagePage.write(tmpOutputFilename->getCString());
    
    delete tmpOutputFilename;
}

// NOTE: void gs_create_pages(PDFDoc *doc)
// function to generate images out of the PDF, using GhostScript... way better than ImageOutputDev
// This is the line within trent: 
// $command = $this->gs_command . " -q -dNOPROMPT -dNOPAUSE -sDEVICE=jpeg -dJPEGQ=70 -r".$this->dpi." \
            -dFirstPage=1 -dLastPage=1 \
            -sOutputFile=".$this->tmp_image." ".$this->tmp_pdf." -c quit";
            
GBool gs_create_pages(PDFDoc *doc, GString *PDFFilename, int dpi) {
    GString *gsCommand;
    GString *tmpOutputFilename;
    
    tmpOutputFilename = get_output_filename(PDFFilename);
    // thumbnail name...
    if ( onlyThumbnails ) {
        tmpOutputFilename->append("_%d_thumb");
    } else {
        // only requesting one page, so use that number for the filename...
        if ( firstPage == lastPage ) {
            tmpOutputFilename->append("_");
            ostringstream sstream;
            sstream << lastPage;
            string tmpNumber = sstream.str();
            tmpOutputFilename->append(tmpNumber.c_str());
        } 
        else {
            // use 1 to n..... end of document
            tmpOutputFilename->append("_%d");
        }
    }
    // add the extension..
    tmpOutputFilename->append(".jpg");
    
    // Double to string type comvertion..  move this to a Template
    ostringstream sstream;
    sstream << dpi;
    string tmpDPI = sstream.str();

    // startng the creation of GS commandline....
    gsCommand = new GString(GS);
    // image quality
    gsCommand->append(" -q -dNOPROMPT -dNOPAUSE -dBATCH -dBGPrint=true -dOpenOutputFile=true -sDEVICE=jpeg");
    if( onlyThumbnails ) {
        gsCommand->append(" -dJPEGQ=30");
    } else {
        gsCommand->append(" -dJPEGQ=60");
    }
    // gsCommand->append(" -dJPEGQ=70");
    gsCommand->append(" -r");
    gsCommand->append(tmpDPI.c_str());
    // antialias images... 
    gsCommand->append(" -dTextAlphaBits=");
    gsCommand->append(gsCommand->fromInt(antialiasing));
    gsCommand->append(" -dGraphicsAlphaBits=");
    gsCommand->append(gsCommand->fromInt(antialiasing));
    
    // add first and last pages if provided otherwise generate all...
    if ( lastPage > 0 ) {
        gsCommand->append(" -dFirstPage=");
        gsCommand->append(gsCommand->fromInt(firstPage));
        gsCommand->append(" -dLastPage=");
        gsCommand->append(gsCommand->fromInt(lastPage));
    }
    // output...
    gsCommand->append(" -sOutputFile=");
    gsCommand->append(tmpOutputFilename->getCString());
    gsCommand->append(" ");
    gsCommand->append(PDFFilename->getCString());
    gsCommand->append(" -c quit");
    
    if(debugging)
        fprintf(stderr, "GS String: %s\n", gsCommand->getCString());
    
    if(system(gsCommand->getCString()) && !errQuiet) {
        delete gsCommand;
        delete tmpOutputFilename;
        error(-1, "gviewer ERROR: Failed to execute ghostScript... make sure it's installed\n");
        return gFalse;
    }
    return gTrue;
}

//  returns PDFFilename without the extension with full path. \
    Example; in: /dir/foo.pdf | out: /dir/foo
GString *getPathFromPDFFilename(GString *PDFFilename, GString *extension){
    char *tmpStr;
    tmpStr = PDFFilename->getCString() + (PDFFilename->getLength() - extension->getLength());
    if(!strcmp(tmpStr, extension->getCString())) {
        return new GString(PDFFilename->getCString(), PDFFilename->getLength() - extension->getLength());
    }
    return new GString();
}

//  this returns the file name from the full-path PDF filename provided \
    it returns just the filename without path nor extension.  Example;  in: /dir/foo.pdf | out: foo
GString *getOutputFilename(GString *PDFFilename) {

    char *slashPointer, *endPointer, *tmpStr;
    GString *imagesFilename;
    int startPos, endPos, strLength;
    
    tmpStr = PDFFilename->getCString() + (PDFFilename->getLength() - 4);
    if(!strcmp(tmpStr, ".pdf") || (!strcmp(tmpStr, ".PDF"))) {
        imagesFilename = new GString(PDFFilename->getCString(), PDFFilename->getLength() - 4);
    }
    
    endPos = imagesFilename->getLength();
    
    // return new string if "/" is found other wise return an empty string
    if((slashPointer = strrchr(imagesFilename->getCString(), '/'))) {
        startPos = (slashPointer-imagesFilename->getCString()+1);
        // endPointer = strrchr(imagesFilename->getCString(), '\0');
        // endPos = (endPointer-imagesFilename->getCString());
        return new GString(imagesFilename, startPos, endPos);
    } 
    return new GString();
}

void get_pages_to_highlight(vector<int> *pagesToHighlight) {
    size_t i = 0;
    GBool found;
    int page;
    
    while( i < rectangleHighlights.size() ) {
        found = gFalse;
        page = pageRec(&rectangleHighlights[i]);
        // store always the first item
        if(pagesToHighlight->size() == 0) {
            found = gFalse;
        } else {
            for(size_t j = 0; j < pagesToHighlight->size(); ++j) {
                if( pagesToHighlight->at(j) == page ) {
                    found = gTrue;
                    break;
                }
            }
        }
        if(!found)
            pagesToHighlight->push_back(page);
        i++;
    }
}
// REGRESA
void get_page_thumbnail(int page, GString *PDFFilename) {
    GString *tmpOutputFilename;
    GString *thumbsFilename;
        
    // compose the filename of the page to highlight
    tmpOutputFilename = get_output_filename(PDFFilename);
    tmpOutputFilename->append("_");
    tmpOutputFilename->append(tmpOutputFilename->fromInt(page));
    tmpOutputFilename->append(".jpg");
    
    try{
        Image ImagePage;
        ImagePage.read(tmpOutputFilename->getCString());
        ImagePage.resolutionUnits(PixelsPerInchResolution); 
        ImagePage.density(Geometry(72,72));
        // Create thubnails...
        thumbsFilename = getPathFromPDFFilename(tmpOutputFilename, new GString(".jpg"));
        thumbsFilename->append("_thumb.jpg");
        Image PageThumbnails(ImagePage);
        PageThumbnails.scale(Geometry((PageThumbnails.columns() / scaleThumbsFactor), (PageThumbnails.rows() / scaleThumbsFactor)));
        // write thumbnails...
        PageThumbnails.write(thumbsFilename->getCString());
        
    } catch(Exception &error_){
        cout << error_.what() << endl;
    }
    
    // delete tmpOutputFilename;
    // delete thumbsFilename;
}

GString *get_output_filename(GString *PDFFilename){
    GString *tmpOutputFilename;
    
    // no output provided, use then location were this is running...
    const char comp[256] = "";
    if(strcmp(outputFilename, comp) == 0 ) {
        // use PDF file location and filename for output...
        tmpOutputFilename = getPathFromPDFFilename(PDFFilename, new GString(".pdf"));
    } else {
        // use output filename provided...
        tmpOutputFilename = getPathFromPDFFilename(new GString(outputFilename), new GString("_%d.jpg"));
    }
    // avoid bug in GString... no longer appends things so create new one...
    const char *tmp = tmpOutputFilename->getCString();
    tmpOutputFilename = new GString(tmp);
    delete tmp;

    return tmpOutputFilename;
}

// WORKING: Main function
int main(int argc, char *argv[]) {
    PDFDoc *doc;
    regex_t regex;
    char **wordsList = NULL;
    char *cfgFileName = "";
    GBool OK;
    GBool performSearch = gTrue;
    GString *patternGiven;
    GString *PDFFilename;
    size_t elementsInList = 0;
    int regexError;
    int exitCode;
    int regexCase = 0;
    int numPages;
    vector<int> pagesToHighlight;
    GBool doPageThumb;
    
    // assume there is an error
    exitCode = 99;
    
    OK = parseArgs(argDesc, &argc, argv);
    
    // Check for PATTERN and <PDF-File> arguments.. and check for other options...
    if(!OK || argc < 2 || argc > 3 || printHelp ) {
        fprintf(stderr, "gViewer version %s, based on Xpdf version %s\n", gViewerVersion, xpdfVersion);
        fprintf(stderr, "%s\n", "Copyright 2012 - Gabriel A Hernandez Castellanos | British Universities Film and Video Council");

        // Print help if no ONLY version required
        if( !printVersion )
        {
            printUsage("gviewer", "[PATTERN] <PDF-File>", argDesc);
            fprintf(stderr, "PATTERN is an extended regular expression.\nNO PATTERN will produce only the pages\n\n");
        }
        exitCode = 1;
        exit(exitCode);
    }
    // set some gobals argumensts
    globalParams = new GlobalParams(cfgFileName);
    globalParams->setTextPageBreaks(gTrue);
    globalParams->setErrQuiet(gTrue);
        
    // Do something if at least we have a PDF
    if(argc == 2){
        PDFFilename = new GString(argv[1]);
        patternGiven = new GString();
    } else if (argc == 3) {
        patternGiven = new GString(argv[1]);
        PDFFilename = new GString(argv[2]);
    } else {
        printUsage("gviewer", "PATTERN <PDF-File>", argDesc);
        fprintf(stderr, "PATTERN is an extended regular expression.\n\n");
        exitCode = 2;
        exit(exitCode);
    }
        
    //  if no PATTERN provided only generate pages don't search nor highlight...
    if ( patternGiven->getLength() == 0 )
        performSearch = gFalse;
    
    if( performSearch ) {
        if(ignoreCase)
            regexCase = REG_ICASE;
        // Lets make sure the PATTERN is a valid extended regular expression.
        regexError = regcomp(&regex, patternGiven->getCString(), REG_EXTENDED | regexCase );
        if (regexError) {
            char err_msg[256];
            regerror(regexError, &regex, err_msg, 256);
        
            if(!errQuiet)
                fprintf(stderr, "gviewer ERROR: %s\n", err_msg);
            
            // there is a problem with the RegExp PATTERN
            exitCode = 3;
            exit(exitCode);
        }
    }
    
    // so far some checks passed... get the pdf file....
    doc = new PDFDoc(PDFFilename);
        
    // make sure the file is OK
    if (!doc->isOk()) {
        if (!errQuiet)
            fprintf( stderr, "gviewer ERROR: Could not open PDF File <%s>\n", PDFFilename->getCString() );
        // there is a problem with the PDF File...
        exitCode = 4;
        exit(exitCode);
    }
    
    numPages = doc->getNumPages();
    
    // get page range.. first page should be always < last page
    if(firstPage < 1)
        firstPage = 1;
    if (lastPage < 1 || lastPage > numPages)
        lastPage = numPages;
    if (firstPage > lastPage)
        firstPage = lastPage;
    
    // change the size of the page to be generated
    if ( onlyThumbnails )
        dpi = dpi / scaleThumbsFactor;

    // generate images...
    // Finish if pages were not created
    if(!gs_create_pages(doc, PDFFilename, dpi)) {
        if(!errQuiet)
            fprintf( stderr, "gviewer ERROR: GS Pages could not be generated | <%s>\n", PDFFilename->getCString() );
        exitCode = 5;
        exit(exitCode);
    }
    
    if( debugging )
        fprintf( stderr, "first page: %d |  laste page: %d\n", firstPage, lastPage);
    
    // run a regular expression search in the text of the document for the terms...
    if (performSearch) {
        
        if( search_in_document(doc, &regex, &wordsList, &elementsInList) > 0 ) {
            // perform search of elements found...
            for(size_t i = 0; i < elementsInList; ++i) {
                // build struct with data
                get_terms_coordinates(doc, wordsList[i]);
            }
        }

        get_pages_to_highlight(&pagesToHighlight);

        // highlight terms
        for(size_t i = 0; i < pagesToHighlight.size(); ++i) {
            highlight_found_text(doc, pagesToHighlight.at(i), PDFFilename);
        }
    }
    
    // generate the rest of the thumbnails that the highlighted did not.
    if ( ! onlyThumbnails ) {
        
        int pagesToProcess = 1;
        if( lastPage > 0 ) {
            if ( firstPage != lastPage )
                pagesToProcess = ( lastPage - firstPage ) + 1;
        } else {
            pagesToProcess = numPages;
        }
        
        for(int page = 0; page < pagesToProcess; ++page) {
            doPageThumb = gTrue;
            for(size_t i = 0; i < pagesToHighlight.size(); ++i) {
                 if(pagesToHighlight.at(i) == (page+1)) {
                     doPageThumb = gFalse;
                     break;
                }
            }
            if(doPageThumb) {
                // check that the thumbnail has not been created already.
                if (firstPage == lastPage)
                    get_page_thumbnail(lastPage, PDFFilename);
                else
                    get_page_thumbnail(page+1, PDFFilename);
            }
        }
    }
    
    // clean up...
    if(doc) delete doc;
    if(wordsList) delete wordsList;
    if(patternGiven) delete patternGiven;
    
    // check for memory leaks
    Object::memCheck(stderr);
    gMemReport(stderr);
    
    exitCode = 0;
    return exitCode;
}
