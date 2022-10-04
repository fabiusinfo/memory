/**************************************************************************/
/*                                                                        */
/*                  Jens                                                  */
/*                                                                        */
/**************************************************************************/
/* Allows to display Jacobian Information of a mesh in .m3d format        */
/* Written by Claudio Lobos (clobos@inf.utfsm.cl) 2015					  */
/* UNIVERSIDAD TECNICA FEDERICO SANTA MARIA								  */
/**************************************************************************/

/*
 <JENS: this program computes the Element Normalized Scaled Jacobian (Jens)>
 
 Copyright (C) <2015>  <Claudio Lobos>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/gpl.txt>
 */

#include "Element.h"

Element::Element(){
    type = 'E';
}

Element::Element(vector<int> &pts) {
    points.assign(pts.size(),0);
    for (unsigned int i=0; i<pts.size(); i++) {
        points[i] = pts[i];
    }
}

Element::~Element(){
    
}

double Element::getEdgeAspectRatio(vector<Point3D> &points) {
    cout << "not imeplemented\n";
    return 0;
}

double Element::getElementJS(vector<Point3D> &pts){
    
    double worst = 2;
    
    vector<vector<unsigned int> > tets = getTetras();
    
    for (unsigned int i=0; i<tets.size(); i++) {
        
        Point3D v0 = pts[tets[i][1]] - pts[tets[i][0]];
        Point3D v1 = pts[tets[i][2]] - pts[tets[i][0]];
        Point3D v2 = pts[tets[i][3]] - pts[tets[i][0]];
        
        v0.normalize();
        v1.normalize();
        v2.normalize();
        double q = (v0.cross(v1)).dot(v2);
        if (q<worst) {
            worst = q;
        }
    }
    
    return worst;
    
}

double Element::getElementJENS(vector<Point3D> &pts){

    double worst = 2;
    
    vector<vector<unsigned int> > tets = getTetras();
    
    for (unsigned int i=0; i<tets.size(); i++) {
        
        Point3D v0 = pts[tets[i][1]] - pts[tets[i][0]];
        Point3D v1 = pts[tets[i][2]] - pts[tets[i][0]];
        Point3D v2 = pts[tets[i][3]] - pts[tets[i][0]];
        
        v0.normalize();
        v1.normalize();
        v2.normalize();
        double q = (v0.cross(v1)).dot(v2);

        if (getCharType()!='H') {
            if (q>jens_constant[i]) {
                q = 1 + jens_constant[i] - q;
            }
            else {
                if (q<(jens_constant[i]*-1)) {
                    q = -1 - jens_constant[i] - q;
                }
                else {
                    q = q/jens_constant[i];
                }
            }
        }
        
        if (q<worst) {
            worst = q;
        }
    }
    
    return worst;
}

vector<double> Element::getJS(vector<Point3D> &pts){
    
    vector<vector<unsigned int> > tets = getTetras();
    vector<double> js(tets.size(),0);
    
    for (unsigned int i=0; i<tets.size(); i++) {
        
        Point3D v0 = pts[tets[i][1]] - pts[tets[i][0]];
        Point3D v1 = pts[tets[i][2]] - pts[tets[i][0]];
        Point3D v2 = pts[tets[i][3]] - pts[tets[i][0]];
        
        v0.normalize();
        v1.normalize();
        v2.normalize();
        
        js[i] = (v0.cross(v1)).dot(v2);
    }
    
    return js;
}

vector<double> Element::getJENS(vector<Point3D> &pts){
    
    vector<vector<unsigned int> > tets = getTetras();
    vector<double> jens(tets.size(),0);
    
    for (unsigned int i=0; i<tets.size(); i++) {
        
        Point3D v0 = pts[tets[i][1]] - pts[tets[i][0]];
        Point3D v1 = pts[tets[i][2]] - pts[tets[i][0]];
        Point3D v2 = pts[tets[i][3]] - pts[tets[i][0]];
        
        v0.normalize();
        v1.normalize();
        v2.normalize();
        double q = (v0.cross(v1)).dot(v2);
        
        if (getCharType()!='H') {
            if (q>jens_constant[i]) {
                q = 1 + jens_constant[i] - q;
            }
            else {
                if (q<(jens_constant[i]*-1)) {
                    q = -1 - jens_constant[i] - q;
                }
                else {
                    q = q/jens_constant[i];
                }
            }
        }
		
        jens[i] = q;
    }
	
    return jens;
}

std::ostream& operator<<(std::ostream& o, Element &e){
    vector<unsigned int> points = e.getPoints();
    o << e.getCharType();
    for (unsigned int i=0; i<points.size(); i++) {
        o << " " << points[i];
    }
    return o;
}


