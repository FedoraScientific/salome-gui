/***************************************************************************
**  Class:   GLViewer_Geom
**  Descr:   
**  Module:  GLViewer
**  Created: UI team, 16.11.04
****************************************************************************/

//#include <GLViewerAfx.h>
#include <GLViewer_Geom.h>

#define FAR_POINT 1e10  // Value used as a "very distant" co-ordinate
#define TOLERANCE 1e-3

//================================================================
// Function : GLViewer_Segment
// Purpose  : constructs a real segment bounded by two points
//================================================================
GLViewer_Segment::GLViewer_Segment( const GLViewer_Pnt& thePnt1, 
                                    const GLViewer_Pnt& thePnt2 )
: myPnt1( thePnt1 ), 
  myPnt2( thePnt2 )
{
  myA = myPnt1.y() - myPnt2.y();
  myB = myPnt2.x() - myPnt1.x();
  myC = myPnt1.x() * myPnt2.y() - myPnt2.x() * myPnt1.y();
}

//================================================================
// Function : GLViewer_Segment
// Purpose  : constructs a ray starting at <thePnt> and directed
//            along positive X axis direction (or Y axis if vertical )
//================================================================
GLViewer_Segment::GLViewer_Segment( const GLViewer_Pnt& thePnt, 
                                    const GLfloat theA, 
                                    const GLfloat theB,
                                    const GLfloat theC )
: myPnt1( thePnt ),
  myA( theA ),
  myB( theB ), 
  myC( theC )
{
  if ( fabs( myB ) < TOLERANCE )
    myPnt2 = GLViewer_Pnt( myPnt1.x(), FAR_POINT );
  else
    myPnt2 = GLViewer_Pnt( FAR_POINT, - myA / myB * FAR_POINT - myC / myB );
}

//================================================================
// Function : GLViewer_Segment
// Purpose  : destructor, does nothing
//================================================================
GLViewer_Segment::~GLViewer_Segment()
{
}

//================================================================
// Function : HasIntersection
// Purpose  : detects intersection with segment <theOther>
//================================================================
bool GLViewer_Segment::HasIntersection( const GLViewer_Segment& theOther ) const
{
  bool aRes = false;
  GLfloat aDiv = myA * theOther.myB - myB * theOther.myA;
  if ( fabs( aDiv ) > TOLERANCE )
  {
    GLfloat aX  = ( myB * theOther.myC - theOther.myB * myC ) / aDiv;
    GLfloat aX11 = myPnt1.x() > myPnt2.x() ? myPnt2.x() : myPnt1.x();
    GLfloat aX12 = myPnt1.x() > myPnt2.x() ? myPnt1.x() : myPnt2.x();
    GLfloat aX21 = theOther.myPnt1.x() > theOther.myPnt2.x() ? theOther.myPnt2.x() : theOther.myPnt1.x();
    GLfloat aX22 = theOther.myPnt1.x() > theOther.myPnt2.x() ? theOther.myPnt1.x() : theOther.myPnt2.x();

    GLfloat aY  = ( myC * theOther.myA - theOther.myC * myA ) / aDiv;
    GLfloat aY11 = myPnt1.y() > myPnt2.y() ? myPnt2.y() : myPnt1.y();
    GLfloat aY12 = myPnt1.y() > myPnt2.y() ? myPnt1.y() : myPnt2.y();
    GLfloat aY21 = theOther.myPnt1.y() > theOther.myPnt2.y() ? theOther.myPnt2.y() : theOther.myPnt1.y();
    GLfloat aY22 = theOther.myPnt1.y() > theOther.myPnt2.y() ? theOther.myPnt1.y() : theOther.myPnt2.y();

    if ( fabs( aX11 - aX12 ) > TOLERANCE )
      aRes = aX11 < aX && aX < aX12;
    else
      aRes = aY11 < aY && aY < aY12;

    if ( aRes )
    {
      if ( fabs( aX21 - aX22 ) > TOLERANCE )
        aRes = aX21 < aX && aX < aX22;
      else
        aRes = aY21 < aY && aY < aY22;
    }
  }

  return aRes;
}

//================================================================
// Function : GLViewer_Poly
// Purpose  : constructs a closed polygon from the given ordered list of points
//================================================================
GLViewer_Poly::GLViewer_Poly( const GLViewer_PntList* thePoints )
: myPoints( (GLViewer_PntList*)thePoints )
{
}

//================================================================
// Function : ~GLViewer_Poly
// Purpose  : destructor, <myPoints> mustn't be deleted here!
//================================================================
GLViewer_Poly::~GLViewer_Poly()
{
}

//================================================================
// Function : IsIn
// Purpose  : returns true if <thePnt> lies within this polygon
//================================================================
bool GLViewer_Poly::IsIn( const GLViewer_Pnt& thePnt ) const
{
  if ( !myPoints )
    return false;

  //cout << thePnt.x() << endl;
  //cout << thePnt.y() << endl << endl;

  int aNbInter = 0;
  GLViewer_Segment aRay( thePnt, 0., 1., -thePnt.y() );

  GLViewer_PntList::const_iterator it1 = myPoints->begin();
  GLViewer_PntList::const_iterator it2 = myPoints->begin();
  ++it2;
  for ( ; it1 != myPoints->end(); ++it1, ++it2 )
  {
    if ( it2 == myPoints->end() )
      it2 = myPoints->begin();
    
    if ( aRay.HasIntersection( GLViewer_Segment( *it1, *it2 ) ) )
      aNbInter++;
  }

  return ( aNbInter % 2 == 1 );
}
/*
//================================================================
// Function : IsIn
// Purpose  : returns true if <thePnt> lies within this polygon
//================================================================
bool GLViewer_Poly::IsIn( const GLViewer_Pnt& thePnt, const float tolerance ) const
{
  if ( !myPoints )
    return false;

  float x = thePnt.x();
  float y = thePnt.y();
  bool res = false;
  
  GLViewer_Pnt p1( x - tolerance, y - tolerance );
  GLViewer_Pnt p2( x - tolerance, y + tolerance );
  GLViewer_Pnt p3( x + tolerance, y - tolerance );
  GLViewer_Pnt p4( x + tolerance, y + tolerance );

  res = ( IsInPnt( thePnt ) ||
          IsInPnt( p1 ) || IsInPnt( p2 ) || IsInPnt( p3 ) || IsInPnt( p4 ) );

  return res;
}
*/
//================================================================
// Function : IsCovers
// Purpose  : returns true if <thePoly> covers this polygon
//================================================================
bool GLViewer_Poly::IsCovers( const GLViewer_Poly& thePoly ) const
{
    if ( !myPoints || !thePoly.Count() )
        return false;

    GLViewer_PntList::const_iterator it = myPoints->begin();
    
    for ( ; it != myPoints->end(); ++it )
    {
        if( !thePoly.IsIn( *it ) )
            return false;
    }

    return true;
}

//================================================================
// Function : IsCovers
// Purpose  : returns true if <theRect> covers this polygon
//================================================================
bool GLViewer_Poly::IsCovers( const GLViewer_Rect& theRect ) const
{
    if ( !myPoints ) //needs check for <theRect>
        return false;

    GLViewer_PntList aList;    
    GLViewer_PntList::iterator it = aList.begin();
    
    aList.insert( it, GLViewer_Pnt( theRect.left(), theRect.top() ) );
    aList.insert( it, GLViewer_Pnt( theRect.right(), theRect.top() ) );
    aList.insert( it, GLViewer_Pnt( theRect.right(), theRect.bottom() ) );
    aList.insert( it, GLViewer_Pnt( theRect.left(), theRect.bottom() ) );

    return IsCovers( GLViewer_Poly( &aList ) );
}

//================================================================
// Function : HasIntersection
// Purpose  : looks for any 
//================================================================
bool GLViewer_Poly::HasIntersection( const GLViewer_Segment& theSegment ) const
{
  if ( !myPoints )
    return false;

  bool aRes = false;
  GLViewer_PntList::const_iterator it1 = myPoints->begin();
  GLViewer_PntList::const_iterator it2 = myPoints->begin();
  ++it2;
  for ( ; !aRes && it1 != myPoints->end(); ++it1, ++it2 )
  {
    if ( it2 == myPoints->end() )
      it2 = myPoints->begin();
    
    aRes = theSegment.HasIntersection( GLViewer_Segment( *it1, *it2 ) );
  }

  return aRes;
}