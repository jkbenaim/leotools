/*************************************************************************
 *   leotools
 *   Copyright (C) 2016 jkbenaim
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <sys/time.h>
#include <sys/stat.h>

#include "endian.h"

int winWidth  = 640;
int winHeight = 480;

Display                 *dpy;
Window                  root;
GLint                   att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
XVisualInfo             *vi;
Colormap                cmap;
XSetWindowAttributes    swa;
Window                  win;
GLXContext              glc;
XWindowAttributes       gwa;
XEvent                  xev;

int frame = 0;
int frametime = 0;
struct timeval tvPrev, tvCur, tvDiff;
Window winChild;
int rootx, rooty, winx, winy;
unsigned int mask_return;
int inWindow = 0;
GLuint tex;
float rotate = 0.0;
int wireframe = 0;

GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat mat_shininess[] = { 50.0 };
GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

struct header *header;
struct model **models;
GLuint *modelDLs;

struct __attribute__((__packed__)) header {
    uint8_t     thumbnail[0x480];
    uint32_t    numModels;
    uint32_t    numVerts;
    uint32_t    numTris;
    uint32_t    modelOffset;
    uint32_t    modelSize;
    uint32_t    texOffset;
    uint32_t    texSize;
    // also a bunch of indecipherable stuff
};

struct __attribute__((__packed__)) model {
    uint32_t vertices;
    uint32_t unk4;
    uint32_t tris;
    uint32_t unkC;
    uint32_t vertDefsOffset;
    uint32_t vertDefsSize;
    uint32_t structBOffset;
    uint32_t structBSize;
    uint32_t triDefsOffset;
    uint32_t triDefsSize;
    uint32_t unk28;
    uint32_t unk2C;
    uint32_t offsetToNextModel;
    uint32_t unk34;
};

struct __attribute__((__packed__)) vert {
    int16_t x;
    int16_t y;
    int16_t z;
    int8_t nx;
    int8_t ny;
    int8_t nz;
    uint8_t unk9;
};

struct __attribute__((__packed__)) tri {
    uint16_t vert1;
    uint16_t vert2;
    uint16_t vert3;
    uint8_t unk6[8];
};


void die( char *msg )
{
    fprintf( stderr, "%s\n", msg );
    exit(1);
}

#ifndef timersub
// needed for IRIX....
void timersub( struct timeval *a, struct timeval *b, struct timeval *res ) {
    res->tv_sec = a->tv_sec - b->tv_sec;
    res->tv_usec = a->tv_usec - b->tv_usec;
    if( res->tv_usec < 0 )
    {
        res->tv_sec --;
        res->tv_usec += 1000000;
    }
}
#endif

void InitGraphics() {
    glEnable( GL_DEPTH_TEST );
    
    // set up projection matrix
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    gluLookAt( 0., 0., 10., 0., 0., 0., 0., 1., 0. );
    
    // lighting
    glShadeModel( GL_SMOOTH );
    glMaterialfv( GL_FRONT, GL_SPECULAR, mat_specular );
    glMaterialfv( GL_FRONT, GL_SHININESS, mat_shininess );
    glLightfv( GL_LIGHT0, GL_POSITION, light_position );
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    
    // scale normal vectors to magnitude 1
    glEnable( GL_NORMALIZE );
}

void DrawModels() {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    
    rotate += 0.1;
//     glRotatef( rotate*0.5, 0.0, 0.0, 1.0 );         // counter-clockwise
//     glRotatef( rotate*1.2, 1.0, 0.0, 0.0 );     // top towards camera
    glRotatef( rotate*2.0, 0.0, 1.0, 0.0 );     // left towards camera
    
    glColor3f( 0.2, 0.2, 0.2 );
    
    if( wireframe )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    
    for(int i=0; i < header->numModels; i++)
        if( glIsList( modelDLs[i] ) )
            glCallList( modelDLs[i] );
    
    glPopMatrix();
    glutSwapBuffers();
}

void LoadFile( char *filename ) {
    FILE *f;
    struct stat sb;
    if( stat( filename, &sb ) == -1 )
        die( "couldn't stat MA3D1 file" );
    
    // Allocate memory for MA3D1 file.
    uint8_t *ma3d1 = (uint8_t*)malloc(sb.st_size);
    if( !ma3d1 )
        die( "couldn't allocate memory to load MA3D1 file" );
    
    // Read MA3D1 file.
    f = fopen( filename, "r" );
    if( !f )
        die( "couldn't open MA3D1 file" );
    if( fread( ma3d1, sb.st_size, 1, f ) != 1 )
        die( "couldn't read MA3D1 file" );
    fclose( f );
    
    // Set up header struct pointer.
    header = (struct header *)ma3d1;
    // Byteswap header.
    header->numModels   = be32toh( header->numModels );
    header->numVerts    = be32toh( header->numVerts );
    header->numTris     = be32toh( header->numTris );
    header->modelOffset = be32toh( header->modelOffset );
    header->modelSize   = be32toh( header->modelSize );
    header->texOffset   = be32toh( header->texOffset );
    header->texSize     = be32toh( header->texSize );

//     printf( "# Models: %d, verts: %d, tris: %d\n",
//         header->numModels,
//         header->numVerts,
//         header->numTris
//     );
//     printf( "# First model data at 0x%X, size %d bytes\n", 
//         header->modelOffset,
//         header->modelSize
//     );
    
    // Allocate an array of model structs.
    models = calloc( header->numModels, sizeof(struct model *) );
    
    // Populate model struct pointers.
    models[0] = (struct model *)(ma3d1 + header->modelOffset);
    for(int i=1; i<header->numModels; i++)
        models[i] = (struct model *)(ma3d1 + header->modelOffset + be32toh(models[i-1]->offsetToNextModel));
    
    // Byteswaps...
    for(int i=0; i<header->numModels; i++) {
        // Byteswap all model defs.
        models[i]->vertices             = be32toh( models[i]->vertices );
        models[i]->unk4                 = be32toh( models[i]->unk4 );
        models[i]->tris                 = be32toh( models[i]->tris );
        models[i]->unkC                 = be32toh( models[i]->unkC );
        models[i]->vertDefsOffset       = be32toh( models[i]->vertDefsOffset );
        models[i]->vertDefsSize         = be32toh( models[i]->vertDefsSize );
        models[i]->structBOffset        = be32toh( models[i]->structBOffset );
        models[i]->structBSize          = be32toh( models[i]->structBSize );
        models[i]->triDefsOffset        = be32toh( models[i]->triDefsOffset );
        models[i]->triDefsSize          = be32toh( models[i]->triDefsSize );
        models[i]->unk28                = be32toh( models[i]->unk28 );
        models[i]->unk2C                = be32toh( models[i]->unk2C );
        models[i]->offsetToNextModel    = be32toh( models[i]->offsetToNextModel );
        models[i]->unk34                = be32toh( models[i]->unk34 );
        
        // Byteswap all vertex defs.
        struct vert *verts = (struct vert *)(ma3d1 + header->modelOffset + models[i]->vertDefsOffset);
        for(int j=0; j<models[i]->vertices; j++ ) {
            verts[j].x = be16toh( verts[j].x );
            verts[j].y = be16toh( verts[j].y );
            verts[j].z = be16toh( verts[j].z );
        }
        
        // Byteswap all tri defs.
        struct tri *tris = (struct tri *)(ma3d1 + header->modelOffset + models[i]->triDefsOffset);
        for(int j=0; j<models[i]->tris; j++ ) {
            tris[j].vert1 = be16toh( tris[j].vert1 );
            tris[j].vert2 = be16toh( tris[j].vert2 );
            tris[j].vert3 = be16toh( tris[j].vert3 );
        }
    }
}

void BuildDisplaylists() {
    modelDLs = calloc( header->numModels, sizeof(GLuint) );
    float scaleFactor = 1/256.0;
    for(int modelNum=0; modelNum < header->numModels; modelNum++) {
        uint8_t *model = (uint8_t *)(models[0]);
        struct vert *verts = (struct vert *)(model + models[modelNum]->vertDefsOffset);
        struct tri  *tris  = (struct tri  *)(model + models[modelNum]->triDefsOffset);
        modelDLs[modelNum] = glGenLists( 1 );
        glNewList( modelDLs[modelNum], GL_COMPILE );
        glBegin( GL_TRIANGLES );
            for(int triNum=0; triNum < models[modelNum]->tris; triNum++) {
                struct tri  *myTri = &tris[triNum];
                struct vert *vert1 = &verts[myTri->vert1];
                struct vert *vert2 = &verts[myTri->vert2];
                struct vert *vert3 = &verts[myTri->vert3];
                
                glNormal3i( vert1->nx,
                            vert1->ny,
                            vert1->nz );
                glVertex3f( vert1->x * scaleFactor,
                            vert1->y * scaleFactor,
                            vert1->z * scaleFactor );
                
                glNormal3i( vert2->nx,
                            vert2->ny,
                            vert2->nz );
                glVertex3f( vert2->x * scaleFactor,
                            vert2->y * scaleFactor,
                            vert2->z * scaleFactor );
                
                glNormal3i( vert3->nx,
                            vert3->ny,
                            vert3->nz );
                glVertex3f( vert3->x * scaleFactor,
                            vert3->y * scaleFactor,
                            vert3->z * scaleFactor );
            }
        glEnd();
        glEndList();
    }
}

void Reshape( GLint width, GLint height ) {
    winWidth  = width;
    winHeight = height;
    glViewport( 0, 0, winWidth, winHeight );
    
    float aspect = (float)winWidth/(float)winHeight;
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0,   // fov
                    aspect, // aspect ratio
                    0.1,    // nearclip
                    100.0   // farclip
    );
}

void Keyboard( unsigned char key, int x, int y ) {
    switch( key ) {
        case 'q':
        case 27:    // esc key
            exit(0);
            break;
        case 'w':
            wireframe = wireframe?0:1;
            break;
        default:
            break;
    }
}

int main( int argc, char *argv[] ) {
    if( argc < 2 )
        die( "need a file" );
    
    printf( "Press w for wireframe, or q to quit.\n" );
    
    LoadFile( argv[1] );
    glutInit( &argc, argv );
    glutInitWindowSize( winWidth, winHeight );
    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
    glutCreateWindow( "ma3d1view" );
    
    glutDisplayFunc( InitGraphics );
    glutReshapeFunc( Reshape );
    glutKeyboardFunc( Keyboard );
    glutIdleFunc( DrawModels );
    
    BuildDisplaylists();
    
    glutMainLoop();
    return 0;
}
