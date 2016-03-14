// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

#include "VTKViewer_PolyDataMapper.h"
#include "VTKViewer_MarkerUtils.h"

#include <utilities.h>

#include <QString>

#include <vtkCellArray.h>
#include <vtkXMLImageDataReader.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>
#include <vtkWindow.h>
#include <vtkRenderWindow.h>
#include <vtkCommand.h>
#include <vtkCellData.h>

#ifndef WIN32
# ifndef GLX_GLXEXT_LEGACY
#  define GLX_GLXEXT_LEGACY
# endif
# include <GL/glx.h>
# include <dlfcn.h>
#else
# include <wingdi.h>
#endif

#ifndef VTK_IMPLEMENT_MESA_CXX
vtkStandardNewMacro(VTKViewer_PolyDataMapper);
#endif

// some definitions for what the polydata has in it
#define VTK_PDPSM_COLORS             0x0001
#define VTK_PDPSM_CELL_COLORS        0x0002
#define VTK_PDPSM_POINT_TYPE_FLOAT   0x0004
#define VTK_PDPSM_POINT_TYPE_DOUBLE  0x0008
#define VTK_PDPSM_NORMAL_TYPE_FLOAT  0x0010
#define VTK_PDPSM_NORMAL_TYPE_DOUBLE 0x0020
#define VTK_PDPSM_OPAQUE_COLORS      0x0040

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

#ifndef GL_ARB_shader_objects
typedef char GLcharARB;
#endif

#ifndef GL_VERTEX_PROGRAM_POINT_SIZE_ARB
#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB  0x8642
#endif

#ifndef GL_VERTEX_SHADER_ARB
#define GL_VERTEX_SHADER_ARB              0x8B31
#endif

#ifndef GL_ARB_point_sprite
#define GL_POINT_SPRITE_ARB               0x8861
#define GL_COORD_REPLACE_ARB              0x8862
#endif

#ifndef GL_ARB_vertex_buffer_object
typedef ptrdiff_t GLsizeiptrARB;

#define GL_ARRAY_BUFFER_ARB               0x8892
#define GL_STATIC_DRAW_ARB                0x88E4
#endif
typedef void (APIENTRYP PFNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
typedef GLhandleARB (APIENTRYP PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
typedef void (APIENTRYP PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRYP PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRYP PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
typedef void (APIENTRYP PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
typedef GLhandleARB (APIENTRYP PFNGLCREATEPROGRAMOBJECTARBPROC) (void);
typedef void (APIENTRYP PFNGLATTACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB obj);
typedef void (APIENTRYP PFNGLLINKPROGRAMARBPROC) (GLhandleARB programObj);
typedef void (APIENTRYP PFNGLUSEPROGRAMOBJECTARBPROC) (GLhandleARB programObj);
typedef GLint (APIENTRYP PFNGLGETATTRIBLOCATIONARBPROC) (GLhandleARB programObj, const GLcharARB *name);
typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERARBPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
typedef void (APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);

typedef GLfloat TBall; 


static PFNGLSHADERSOURCEARBPROC             vglShaderSourceARB             = NULL;
static PFNGLCREATESHADEROBJECTARBPROC       vglCreateShaderObjectARB       = NULL;
static PFNGLCOMPILESHADERARBPROC            vglCompileShaderARB            = NULL;
static PFNGLCREATEPROGRAMOBJECTARBPROC      vglCreateProgramObjectARB      = NULL;
static PFNGLATTACHOBJECTARBPROC             vglAttachObjectARB             = NULL;
static PFNGLLINKPROGRAMARBPROC              vglLinkProgramARB              = NULL;
static PFNGLUSEPROGRAMOBJECTARBPROC         vglUseProgramObjectARB         = NULL;

static PFNGLGENBUFFERSARBPROC               vglGenBuffersARB               = NULL;
static PFNGLBINDBUFFERARBPROC               vglBindBufferARB               = NULL;
static PFNGLBUFFERDATAARBPROC               vglBufferDataARB               = NULL;
static PFNGLDELETEBUFFERSARBPROC            vglDeleteBuffersARB            = NULL;
static PFNGLGETATTRIBLOCATIONARBPROC        vglGetAttribLocationARB        = NULL;
static PFNGLVERTEXATTRIBPOINTERARBPROC      vglVertexAttribPointerARB      = NULL;
static PFNGLENABLEVERTEXATTRIBARRAYARBPROC  vglEnableVertexAttribArrayARB  = NULL;
static PFNGLDISABLEVERTEXATTRIBARRAYARBPROC vglDisableVertexAttribArrayARB = NULL;


#ifndef WIN32
#define GL_GetProcAddress( x )   glXGetProcAddressARB( (const GLubyte*)x )
#else
#define GL_GetProcAddress( x )   wglGetProcAddress( (const LPCSTR)x )
#endif

#ifdef WIN32
  #ifdef max
    #undef max
  #endif
#endif

// ----------------------------------------------- Special Textures -----------------------------------
// texture id for balls drawing
#define BallTextureId 0 

bool InitializeBufferExtensions()
{
  vglShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)GL_GetProcAddress( "glShaderSourceARB" );
  if( !vglShaderSourceARB )
    return false;

  vglCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)GL_GetProcAddress( "glCreateShaderObjectARB" );
  if( !vglCreateShaderObjectARB )
    return false;

  vglCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)GL_GetProcAddress( "glCompileShaderARB" );
  if( !vglCompileShaderARB )
    return false;

  vglCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)GL_GetProcAddress( "glCreateProgramObjectARB" );
  if( !vglCreateProgramObjectARB )
    return false;

  vglAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)GL_GetProcAddress( "glAttachObjectARB" );
  if( !vglAttachObjectARB )
    return false;

  vglLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)GL_GetProcAddress( "glLinkProgramARB" );
  if( !vglLinkProgramARB )
    return false;

  vglUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)GL_GetProcAddress( "glUseProgramObjectARB" );
  if( !vglUseProgramObjectARB )
    return false;

  vglGenBuffersARB = (PFNGLGENBUFFERSARBPROC)GL_GetProcAddress( "glGenBuffersARB" );
  if( !vglGenBuffersARB )
    return false;

  vglBindBufferARB = (PFNGLBINDBUFFERARBPROC)GL_GetProcAddress( "glBindBufferARB" );
  if( !vglBindBufferARB )
    return false;

  vglBufferDataARB = (PFNGLBUFFERDATAARBPROC)GL_GetProcAddress( "glBufferDataARB" );
  if( !vglBufferDataARB )
    return false;

  vglDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)GL_GetProcAddress( "glDeleteBuffersARB" );
  if( !vglDeleteBuffersARB )
    return false;

  vglGetAttribLocationARB = (PFNGLGETATTRIBLOCATIONARBPROC)GL_GetProcAddress( "glGetAttribLocation" );
  if( !vglGetAttribLocationARB )
    return false;

  vglVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARBPROC)GL_GetProcAddress( "glVertexAttribPointer" );
  if( !vglVertexAttribPointerARB )
    return false;

  vglEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)GL_GetProcAddress( "glEnableVertexAttribArray" );
  if(!vglEnableVertexAttribArrayARB)
    return false;

  vglDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)GL_GetProcAddress( "glDisableVertexAttribArray" );

  if(!vglDisableVertexAttribArrayARB)
    return false;

  
  return true;
};

//-----------------------------------------------------------------------------
char* readFromFile( std::string fileName )
{
  FILE* file = fopen( fileName.c_str(), "r" );

  char* content = NULL;
  int count = 0;

  if( file != NULL )
  {
    fseek( file, 0, SEEK_END );
    count = ftell( file );
    rewind( file );

    if( count > 0 )
    {
      content = ( char* )malloc( sizeof( char ) * ( count + 1 ) );
      count = fread( content, sizeof( char ), count, file );
      content[ count ] = '\0';
    }
    fclose( file );
  }

  return content;
}

static bool IsBufferExtensionsInitialized = InitializeBufferExtensions();

//-----------------------------------------------------------------------------
VTKViewer_PolyDataMapper::VTKViewer_PolyDataMapper()
{
  Q_INIT_RESOURCE( VTKViewer );

  this->ExtensionsInitialized     = ES_None;

  this->PointSpriteTexture        = 0;

  this->MarkerEnabled             = false;
  this->MarkerType                = VTK::MT_NONE;
  this->MarkerScale               = VTK::MS_NONE;
  this->MarkerId                  = 0;
  this->BallEnabled               = false;
  this->BallScale                 = 1.0;
  this->VertexProgram             = 0;
}

//-----------------------------------------------------------------------------
VTKViewer_PolyDataMapper::~VTKViewer_PolyDataMapper()
{
  if( PointSpriteTexture > 0 )
    glDeleteTextures( 1, &PointSpriteTexture );
}

//-----------------------------------------------------------------------------
void VTKViewer_PolyDataMapper::InitShader()
{
  std::string fileName = std::string( getenv( "GUI_ROOT_DIR") ) +
                         "/share/salome/resources/gui/Vertex_Program_ARB.txt";

  char* shader = readFromFile( fileName );

  GLhandleARB VertexShader = vglCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );
  vglShaderSourceARB( VertexShader, 1, (const GLcharARB**)&shader, NULL );
  vglCompileShaderARB( VertexShader );

  this->VertexProgram = vglCreateProgramObjectARB();
  vglAttachObjectARB( this->VertexProgram, VertexShader );

  vglLinkProgramARB( this->VertexProgram );
  free( shader );
}


//-----------------------------------------------------------------------------
void VTKViewer_PolyDataMapper::SetMarkerEnabled( bool theMarkerEnabled )
{
  if( this->MarkerEnabled == theMarkerEnabled )
    return;

  this->MarkerEnabled = theMarkerEnabled;
  this->Modified();
}

//-----------------------------------------------------------------------------
// Definition of structures and fuctions used in SetBallEnabled() method
namespace VTK
{
  //----------------------------------------------------------------------------
  vtkSmartPointer<vtkImageData> MakeTexture( const char* theMainTexture, const char* theAlphaTexture ) {
    if( !theMainTexture || !theAlphaTexture )
      return 0;
    
    vtkXMLImageDataReader* aMainReader = vtkXMLImageDataReader::New();
    vtkXMLImageDataReader* anAlphaReader = vtkXMLImageDataReader::New();
    
    aMainReader->SetFileName( theMainTexture );
    anAlphaReader->SetFileName( theAlphaTexture );

    aMainReader->Update();
    anAlphaReader->Update();
    
    vtkImageData* aMainImageData = aMainReader->GetOutput();
    vtkImageData* anAlphaImageData = anAlphaReader->GetOutput();
    
    int* aMainImageSize = aMainImageData->GetDimensions();
    int* anAlphaImageSize = anAlphaImageData->GetDimensions();
    if(aMainImageSize[0] != anAlphaImageSize[0] || aMainImageSize[1] != anAlphaImageSize[1])
      return NULL;

    vtkSmartPointer<vtkImageData> aCompositeImageData = vtkImageData::New();
    aCompositeImageData->Delete();
    
    int aNbCompositeComponents = 4;
    aCompositeImageData->SetDimensions(aMainImageSize);
    aCompositeImageData->AllocateScalars( VTK_UNSIGNED_CHAR, aNbCompositeComponents );
    
    unsigned char* aMainDataPtr = (unsigned char*)aMainImageData->GetScalarPointer();
    unsigned char* anAlphaDataPtr = (unsigned char*)anAlphaImageData->GetScalarPointer();
    unsigned char *aCompositeDataPtr = (unsigned char * )aCompositeImageData->GetScalarPointer();

    int aNbMainComponents = aMainImageData->GetNumberOfScalarComponents();
    int aNbAlphaComponents = anAlphaImageData->GetNumberOfScalarComponents();
    int aCompositeSize = aMainImageSize[0] * aMainImageSize[1] * aNbCompositeComponents;

    int aMainId = 0, anAlphaId = 0, aCompositeId = 0;
    for(; aCompositeId < aCompositeSize;) {
      aCompositeDataPtr[aCompositeId] = aMainDataPtr[aMainId];
      aCompositeDataPtr[aCompositeId + 1] = aMainDataPtr[aMainId + 1];
      aCompositeDataPtr[aCompositeId + 2] = aMainDataPtr[aMainId + 2];
      aCompositeDataPtr[aCompositeId + 3] = anAlphaDataPtr[anAlphaId];

      aMainId += aNbMainComponents;
      anAlphaId += aNbAlphaComponents;
      aCompositeId += aNbCompositeComponents;
    }
    aMainReader->Delete();
    anAlphaReader->Delete();
    return aCompositeImageData;
  }  
}

//-----------------------------------------------------------------------------
bool VTKViewer_PolyDataMapper::GetBallEnabled()
{
  return this->BallEnabled;
}
//-----------------------------------------------------------------------------
void VTKViewer_PolyDataMapper::SetBallEnabled( bool theBallEnabled )
{ 
  if( this->BallEnabled == theBallEnabled )
    return;
  else 
    this->BallEnabled = theBallEnabled;

  if(!this->BallEnabled) {
    this->ImageData = NULL;
  }

  if(this->BallEnabled) {
    if(this->SpecialTextures.find(BallTextureId) == SpecialTextures.end()){
      QString aMainTexture  = getenv( "GUI_ROOT_DIR" );
      aMainTexture.append("/share/salome/resources/gui/sprite_texture.vti");
      
      QString anAlphaTexture = getenv( "GUI_ROOT_DIR" );
      anAlphaTexture.append( "/share/salome/resources/gui/sprite_alpha.vti" );
      vtkSmartPointer<vtkImageData> aTextureValue = VTK::MakeTexture( aMainTexture.toLatin1().constData(), anAlphaTexture.toLatin1().constData() );
      this->SpecialTextures[BallTextureId] = aTextureValue;
    }
    this->ImageData = this->SpecialTextures[BallTextureId];
  }
  this->Modified();
}

//-----------------------------------------------------------------------------
double VTKViewer_PolyDataMapper::GetBallScale()
{
  return this->BallScale;
}
//-----------------------------------------------------------------------------
void VTKViewer_PolyDataMapper::SetBallScale( double theBallScale )
{
  if( this->BallScale == theBallScale )
    return;
  this->BallScale = theBallScale;
}

//-----------------------------------------------------------------------------
void VTKViewer_PolyDataMapper::SetMarkerStd( VTK::MarkerType theMarkerType, VTK::MarkerScale theMarkerScale )
{
  if( this->MarkerType == theMarkerType && this->MarkerScale == theMarkerScale )
    return;

  this->MarkerType = theMarkerType;
  this->MarkerScale = theMarkerScale;

  if( this->MarkerType == VTK::MT_NONE || this->MarkerType == VTK::MT_USER ) {
    this->ImageData = NULL;
    this->Modified();
    return;
  }

  int aMarkerType = (int)this->MarkerType;
  int aMarkerScale = (int)this->MarkerScale;

  int anId = (int)VTK::MS_70 * aMarkerType + aMarkerScale;

  if( this->StandardTextures.find( anId ) == this->StandardTextures.end() )
  {
    QString aFileName = QString( ":/textures/texture%1.dat" ).arg( aMarkerType );
    VTK::MarkerTexture aMarkerTexture;
    if( VTK::LoadTextureData( aFileName, theMarkerScale, aMarkerTexture ) )
      this->StandardTextures[ anId ] = VTK::MakeVTKImage( aMarkerTexture );
  }

  this->ImageData = this->StandardTextures[ anId ];
  this->Modified();
}

//-----------------------------------------------------------------------------
void VTKViewer_PolyDataMapper::SetMarkerTexture( int theMarkerId, VTK::MarkerTexture theMarkerTexture )
{
  if( this->MarkerType == VTK::MT_USER && this->MarkerId == theMarkerId )
    return;

  this->MarkerType = VTK::MT_USER;
  this->MarkerId = theMarkerId;

  if( this->CustomTextures.find( theMarkerId ) == this->CustomTextures.end() )
    this->CustomTextures[ theMarkerId ] = VTK::MakeVTKImage( theMarkerTexture );

  this->ImageData = this->CustomTextures[ theMarkerId ];
  this->Modified();
}

//-----------------------------------------------------------------------------
VTK::MarkerType VTKViewer_PolyDataMapper::GetMarkerType()
{
  return this->MarkerType;
}

//-----------------------------------------------------------------------------
VTK::MarkerScale VTKViewer_PolyDataMapper::GetMarkerScale()
{
  return this->MarkerScale;
}

//-----------------------------------------------------------------------------
int VTKViewer_PolyDataMapper::GetMarkerTexture()
{
  return this->MarkerId;
}

//-----------------------------------------------------------------------------
int VTKViewer_PolyDataMapper::InitExtensions()
{
  char* ext = (char*)glGetString( GL_EXTENSIONS );
  if( !IsBufferExtensionsInitialized ||
      strstr( ext, "GL_ARB_point_sprite" ) == NULL ||
      strstr( ext, "GL_ARB_vertex_buffer_object" ) == NULL ||
      strstr( ext, "GL_ARB_shader_objects") == NULL )
  {
    MESSAGE("Initializing ARB extensions failed");
    return ES_Error;
  }

  if( this->BallEnabled )
    this->InitShader();

  return ES_Ok;
}

//-----------------------------------------------------------------------------
void VTKViewer_PolyDataMapper::InitPointSprites()
{
  glEnable( GL_POINT_SPRITE_ARB );
  glEnable( GL_VERTEX_PROGRAM_POINT_SIZE_ARB );

  glPushAttrib( GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT );

  glDepthFunc( GL_LEQUAL );
  glEnable( GL_DEPTH_TEST );

  glEnable( GL_ALPHA_TEST );
  if(!this->BallEnabled) {
    glAlphaFunc( GL_GREATER, 0.0 );
  }
  else { 
    glAlphaFunc( GL_GREATER, 0.5 );
  }

  glDisable( GL_LIGHTING );

  glDisable( GL_COLOR_MATERIAL );
}

//-----------------------------------------------------------------------------
void VTKViewer_PolyDataMapper::CleanupPointSprites()
{
  glPopAttrib();

  glDisable( GL_VERTEX_PROGRAM_POINT_SIZE_ARB );
  glDisable( GL_POINT_SPRITE_ARB );
}

//-----------------------------------------------------------------------------
void VTKViewer_PolyDataMapper::InitTextures()
{
  if( !this->ImageData.GetPointer() )
    return;

  glEnable( GL_TEXTURE_2D );
  if( this->PointSpriteTexture == 0 ) {
    glGenTextures( 1, &this->PointSpriteTexture );
  }
  glBindTexture( GL_TEXTURE_2D, this->PointSpriteTexture );
  glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  
  if(this->BallEnabled) {
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  } else {
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  }

  int* aSize = this->ImageData->GetDimensions();
  unsigned char* dataPtr = (unsigned char*)this->ImageData->GetScalarPointer();
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, aSize[0], aSize[1], 0,
                GL_RGBA, GL_UNSIGNED_BYTE, dataPtr );
}

//-----------------------------------------------------------------------------
void VTKViewer_PolyDataMapper::RenderPiece( vtkRenderer* ren, vtkActor* act )
{
  bool isUsePointSprites = (this->MarkerEnabled && this->MarkerType != VTK::MT_NONE) || 
    this->BallEnabled;
  if( isUsePointSprites )
  {
    if( this->ExtensionsInitialized == ES_None )
      this->ExtensionsInitialized = this->InitExtensions();
    this->InitPointSprites();
    this->InitTextures();
  }

  if(!this->BallEnabled || this->ExtensionsInitialized != ES_Ok) {
    MAPPER_SUPERCLASS::RenderPiece( ren, act );
    if( isUsePointSprites )
      this->CleanupPointSprites();
    glBindTexture( GL_TEXTURE_2D, 0 );
  } else {
    vtkIdType numPts;
    vtkPolyData *input= this->GetInput();

    //
    // make sure that we've been properly initialized
    //
    if (ren->GetRenderWindow()->CheckAbortStatus())
      return;

    if ( input == NULL )
    {
      vtkErrorMacro(<< "No input!");
      return;
    }
    else
    {
      this->InvokeEvent(vtkCommand::StartEvent,NULL);
      this->Update();
      this->InvokeEvent(vtkCommand::EndEvent,NULL);
      numPts = input->GetNumberOfPoints();
    }

    if (numPts == 0)
    {
      vtkDebugMacro(<< "No points!");
      return;
    }

    // make sure our window is current
    ren->GetRenderWindow()->MakeCurrent();


    vglUseProgramObjectARB( this->VertexProgram );

    //
    // if something has changed regenerate colors and display lists
    // if required
    //
    int noAbort=1;
    if ( this->GetMTime() > this->BuildTime ||
	 input->GetMTime() > this->BuildTime ||
	 act->GetProperty()->GetMTime() > this->BuildTime ||
	 ren->GetRenderWindow() != this->LastWindow)
    {
      // sets this->Colors as side effect
      this->MapScalars( act->GetProperty()->GetOpacity() );

      if (!this->ImmediateModeRendering &&
	  !this->GetGlobalImmediateModeRendering())
      {
	this->ReleaseGraphicsResources(ren->GetRenderWindow());
	this->LastWindow = ren->GetRenderWindow();

	// get a unique display list id
	this->ListId = glGenLists(1);
	glNewList(this->ListId,GL_COMPILE);

	noAbort = this->Draw(ren,act);
	glEndList();

	// Time the actual drawing
	this->Timer->StartTimer();
	glCallList(this->ListId);
	this->Timer->StopTimer();
      }
      else
      {
	this->ReleaseGraphicsResources(ren->GetRenderWindow());
	this->LastWindow = ren->GetRenderWindow();
      }
      if (noAbort)
	this->BuildTime.Modified();
    }
    // if nothing changed but we are using display lists, draw it
    else
    {
      if (!this->ImmediateModeRendering &&
	  !this->GetGlobalImmediateModeRendering())
      {
	// Time the actual drawing
	this->Timer->StartTimer();
	glCallList(this->ListId);
	this->Timer->StopTimer();
      }
    }

    // if we are in immediate mode rendering we always
    // want to draw the primitives here
    if (this->ImmediateModeRendering ||
	this->GetGlobalImmediateModeRendering())
    {
      // sets this->Colors as side effect
      this->MapScalars( act->GetProperty()->GetOpacity() );

      // Time the actual drawing
      this->Timer->StartTimer();
      this->Draw(ren,act);
      this->Timer->StopTimer();
    }

    this->TimeToDraw = (float)this->Timer->GetElapsedTime();

    // If the timer is not accurate enough, set it to a small
    // time so that it is not zero
    if ( this->TimeToDraw == 0.0 )
      this->TimeToDraw = 0.0001;

    vglUseProgramObjectARB( 0 );
    this->CleanupPointSprites();
    glBindTexture( GL_TEXTURE_2D, 0 );
  }  
}

//-----------------------------------------------------------------------------
// Definition of structures and fuctions used in Draw() method
namespace VTK
{
  //-----------------------------------------------------------------------------
  struct TVertex
  {
    GLfloat r, g, b, a;
    GLfloat vx, vy, vz;
  };

  //-----------------------------------------------------------------------------
  struct TColorFunctorBase
  {
    double myAlpha;

    TColorFunctorBase( vtkProperty* theProperty )
    {
      myAlpha = theProperty->GetOpacity();
    }

    virtual
    void
    get( TVertex& theVertex, vtkIdType thePointId, vtkIdType theCellId ) = 0;
  };

  //-----------------------------------------------------------------------------
  struct TPropertyColor : TColorFunctorBase
  {
    double myColor[3];

    TPropertyColor( vtkProperty* theProperty ):
      TColorFunctorBase( theProperty )
    {
      theProperty->GetColor( myColor );
    }

    virtual
    void
    get( TVertex& theVertex, vtkIdType thePointId, vtkIdType theCellId )
    {
      theVertex.r = myColor[0];
      theVertex.g = myColor[1];
      theVertex.b = myColor[2];
      theVertex.a = myAlpha;
    }
  };

  //-----------------------------------------------------------------------------
  struct TColors2Color : TColorFunctorBase
  {
    vtkUnsignedCharArray* myColors;

    TColors2Color( vtkProperty* theProperty,
                   vtkUnsignedCharArray* theColors ):
      TColorFunctorBase( theProperty ),
      myColors( theColors )
    {}

    virtual
    void
    get( TVertex& theVertex, vtkIdType thePointId, vtkIdType theCellId )
    {
      vtkIdType aTupleId = GetTupleId( thePointId, theCellId );
      unsigned char* aColor = myColors->GetPointer( aTupleId << 2 );

      theVertex.r = int( aColor[0] ) / 255.0;
      theVertex.g = int( aColor[1] ) / 255.0;
      theVertex.b = int( aColor[2] ) / 255.0;
      theVertex.a = myAlpha;
    }

    virtual
    vtkIdType
    GetTupleId( vtkIdType thePointId, vtkIdType theCellId ) = 0;
  };

  //-----------------------------------------------------------------------------
  struct TPointColors2Color : TColors2Color
  {
    TPointColors2Color( vtkProperty* theProperty,
                        vtkUnsignedCharArray* theColors ):
      TColors2Color( theProperty, theColors )
    {}

    virtual
    vtkIdType
    GetTupleId( vtkIdType thePointId, vtkIdType theCellId )
    {
      return thePointId;
    }
  };

  //-----------------------------------------------------------------------------
  struct TCellColors2Color : TColors2Color
  {
    TCellColors2Color( vtkProperty* theProperty,
                       vtkUnsignedCharArray* theColors ):
      TColors2Color( theProperty, theColors )
    {}

    virtual
    vtkIdType
    GetTupleId( vtkIdType thePointId, vtkIdType theCellId )
    {
      return theCellId;
    }
  };

  //-----------------------------------------------------------------------------
  template < class TCoordinates >
  void DrawPoints( TCoordinates* theStartPoints,
                   vtkCellArray* theCells,
                   TColorFunctorBase* theColorFunctor,
                   TVertex* theVertexArr,
                   vtkIdType &theCellId,
                   vtkIdType &theVertexId,
                   TBall* theBallArr,
                   vtkDataArray* theDiamArray,
                   double theBallScale )
  {
    vtkIdType* ptIds = theCells->GetPointer();
    vtkIdType* endPtIds = ptIds + theCells->GetNumberOfConnectivityEntries();

    bool mapBalls = false; 
    if(theBallArr && theDiamArray) {
      mapBalls = true;
    }

    while ( ptIds < endPtIds ) {
      vtkIdType nPts = *ptIds;
      ++ptIds;

      while ( nPts > 0 ) {
        TVertex& aVertex = theVertexArr[ theVertexId ];
        vtkIdType aPointId = *ptIds;

        TCoordinates* anOffsetPoints = theStartPoints + 3 * aPointId;
        aVertex.vx = anOffsetPoints[0];
        aVertex.vy = anOffsetPoints[1];
        aVertex.vz = anOffsetPoints[2];

        theColorFunctor->get( aVertex, aPointId, theCellId );

        ++theVertexId;
        ++ptIds; 
        --nPts; 
      }
      
      if(mapBalls){
        theBallArr[theCellId] = (TBall)theDiamArray->GetTuple(theCellId)[0]*theBallScale;
      }

      ++theCellId;
    }
  }

  //-----------------------------------------------------------------------------
  template < class TCoordinates >
  void DrawCellsPoints( vtkPolyData* theInput,
                        vtkPoints* thePoints,
                        TColorFunctorBase* theColorFunctor,
                        TVertex* theVertexArr,
                        TBall* theBallArr,
                        double theBallScale )
  {
    vtkIdType aCellId = 0, aVertexId = 0;

    TCoordinates* aStartPoints = (TCoordinates*)thePoints->GetVoidPointer(0);
    vtkDataArray* aDiams = theInput->GetCellData() ? theInput->GetCellData()->GetScalars() : 0;    

    if ( vtkCellArray* aCellArray = theInput->GetVerts() ) {
      DrawPoints( aStartPoints, aCellArray, theColorFunctor, theVertexArr, aCellId, aVertexId, theBallArr, aDiams, theBallScale );
    }
  
    if ( vtkCellArray* aCellArray = theInput->GetLines() )
      DrawPoints( aStartPoints, aCellArray, theColorFunctor, theVertexArr, aCellId, aVertexId, theBallArr, aDiams, theBallScale );
  
    if ( vtkCellArray* aCellArray = theInput->GetPolys() )
      DrawPoints( aStartPoints, aCellArray, theColorFunctor, theVertexArr, aCellId, aVertexId, theBallArr, aDiams, theBallScale );
  
    if ( vtkCellArray* aCellArray = theInput->GetStrips() )
      DrawPoints( aStartPoints, aCellArray, theColorFunctor, theVertexArr, aCellId, aVertexId, theBallArr, aDiams, theBallScale ); 
  }
} // namespace VTK

//-----------------------------------------------------------------------------
int VTKViewer_PolyDataMapper::Draw( vtkRenderer* ren, vtkActor* act )
{  
  if( (!this->MarkerEnabled || this->MarkerType == VTK::MT_NONE || !this->ImageData.GetPointer()) && !this->BallEnabled)
    return MAPPER_SUPERCLASS::Draw( ren, act );

  vtkUnsignedCharArray* colors = NULL;
  vtkPolyData* input  = this->GetInput();
  vtkPoints* points;
  int noAbort = 1;
  int cellScalars = 0;
  vtkProperty* prop = act->GetProperty();

  points = input->GetPoints();

  if ( this->Colors )
  {
    if(!this->BallEnabled) {
      colors = this->Colors;
      if ( (this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_DATA ||
	    this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA ||
	    !input->GetPointData()->GetScalars() )
	   && this->ScalarMode != VTK_SCALAR_MODE_USE_POINT_FIELD_DATA )
	cellScalars = 1;
    }
  }

  {
    vtkIdType aTotalConnectivitySize = 0;
    vtkIdType aNbCells = 0;

    if ( vtkCellArray* aCellArray = input->GetVerts() ) {
      aTotalConnectivitySize += aCellArray->GetNumberOfConnectivityEntries() - aCellArray->GetNumberOfCells();
      aNbCells += aCellArray->GetNumberOfCells();
    }

    if ( vtkCellArray* aCellArray = input->GetLines() ) {
      aTotalConnectivitySize += aCellArray->GetNumberOfConnectivityEntries() - aCellArray->GetNumberOfCells();
      aNbCells += aCellArray->GetNumberOfCells();
    }

    if ( vtkCellArray* aCellArray = input->GetPolys() ) {
      aTotalConnectivitySize += aCellArray->GetNumberOfConnectivityEntries() - aCellArray->GetNumberOfCells();
      aNbCells += aCellArray->GetNumberOfCells();
    }

    if ( vtkCellArray* aCellArray = input->GetStrips() ) {
      aTotalConnectivitySize += aCellArray->GetNumberOfConnectivityEntries() - aCellArray->GetNumberOfCells();
      aNbCells += aCellArray->GetNumberOfCells();
    }

    if ( aTotalConnectivitySize > 0 ) {
      VTK::TVertex* aVertexArr = new VTK::TVertex[ aTotalConnectivitySize ];
      
      TBall* aBallArray = 0;

      if(this->BallEnabled) {
	aBallArray = new TBall[aNbCells];
      }

      int* aSize = this->ImageData->GetDimensions();
      glPointSize( std::max( aSize[0], aSize[1] ) );

      int aMode = 0; // to remove
      {
        VTK::TColorFunctorBase* aColorFunctor = NULL;
        if( colors && aMode != 1 ) {
          if ( cellScalars )
            aColorFunctor = new VTK::TCellColors2Color( prop, colors );
          else
            aColorFunctor = new VTK::TPointColors2Color( prop, colors );
        } else {
          aColorFunctor = new VTK::TPropertyColor( prop );
        }
        if ( points->GetDataType() == VTK_FLOAT )
          VTK::DrawCellsPoints< float >( input, points, aColorFunctor, aVertexArr, aBallArray, GetBallScale() );
        else
          VTK::DrawCellsPoints< double >( input, points, aColorFunctor, aVertexArr, aBallArray, GetBallScale() );

        delete aColorFunctor;
      }

      if( this->ExtensionsInitialized == ES_Ok ) {
        GLuint aBufferObjectID, aDiamsID = 0;
	GLint attribute_diams = -1;
        vglGenBuffersARB( 1, &aBufferObjectID );
        vglBindBufferARB( GL_ARRAY_BUFFER_ARB, aBufferObjectID );
        
        int anArrayObjectSize = sizeof( VTK::TVertex ) * aTotalConnectivitySize;
        vglBufferDataARB( GL_ARRAY_BUFFER_ARB, anArrayObjectSize, aVertexArr, GL_STATIC_DRAW_ARB );
        
        delete [] aVertexArr;
        
        vglBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
        vglBindBufferARB( GL_ARRAY_BUFFER_ARB, aBufferObjectID );        
        
        glColorPointer( 4, GL_FLOAT, sizeof(VTK::TVertex), (void*)0 );
        glVertexPointer( 3, GL_FLOAT, sizeof(VTK::TVertex), (void*)(4*sizeof(GLfloat)) );
        
        glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_COLOR_ARRAY );

	if(this->BallEnabled) {
	  vglGenBuffersARB( 2, &aDiamsID);
	  vglBindBufferARB( GL_ARRAY_BUFFER_ARB, aDiamsID);

	  int aDiamsSize = sizeof(TBall)*aNbCells;
	  vglBufferDataARB( GL_ARRAY_BUFFER_ARB, aDiamsSize, aBallArray, GL_STATIC_DRAW_ARB);

	  delete [] aBallArray;
	  vglBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
	  vglBindBufferARB( GL_ARRAY_BUFFER_ARB, aDiamsID );

	  attribute_diams = vglGetAttribLocationARB(this->VertexProgram, "diameter");
	  vglEnableVertexAttribArrayARB(attribute_diams);
	  vglBindBufferARB(GL_ARRAY_BUFFER_ARB, aDiamsID);
	  vglVertexAttribPointerARB(
				    attribute_diams,   // attribute
				    1,                 // number of elements per vertex, here (diameter)
				    GL_FLOAT,          // the type of each element
				    GL_FALSE,          // take our values as-is
				    0,                 // no extra data between each position
				    0                  // offset of first element
				    );
	}

        glDrawArrays( GL_POINTS, 0, aTotalConnectivitySize );
        
        glDisableClientState( GL_COLOR_ARRAY );
        glDisableClientState( GL_VERTEX_ARRAY );       
        vglDeleteBuffersARB( 1, &aBufferObjectID );

	if(this->BallEnabled) {
	  vglDisableVertexAttribArrayARB(attribute_diams);
	  vglDeleteBuffersARB( 2, &aDiamsID );
	}

	  } else { // there are no extensions
        glColorPointer( 4, GL_FLOAT, sizeof(VTK::TVertex), aVertexArr );
        glVertexPointer( 3, GL_FLOAT, sizeof(VTK::TVertex), 
                         (void*)((GLfloat*)((void*)(aVertexArr)) + 4));

        glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_COLOR_ARRAY );
        
        glDrawArrays( GL_POINTS, 0, aTotalConnectivitySize );
        
        glDisableClientState( GL_COLOR_ARRAY );
        glDisableClientState( GL_VERTEX_ARRAY );

        delete [] aVertexArr;
      }
    }
  }

  this->UpdateProgress(1.0);
  return noAbort;
}
