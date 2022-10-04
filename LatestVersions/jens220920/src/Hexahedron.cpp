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

#include "Hexahedron.h"

#include <limits.h>
#include <limits>

Hexahedron::Hexahedron(vector<int> &pts):Element(pts) {
    type = 'H';
    jens_constant.assign(8,1);
}

Hexahedron::~Hexahedron(){
    
}

double Hexahedron::getAspectRatio(vector<Point3D> &points) {

    vector<unsigned int> fpoints;
    vector<Point3D> midpoints;
    vector<double> hs;
    double max,min;
    
    midpoints.reserve(6);
    hs.reserve(3);
    
    for(int i=0;i<6;i++){
        fpoints = getFace(i);
        Point3D avg;
        for(int j=0;j<4;j++){
            Point3D aux = points[fpoints[j]];
            avg+=aux;
        }
        avg/=4;
        midpoints.push_back(avg);
    }
    hs.push_back((midpoints[0] - midpoints[5]).Norm());
    hs.push_back((midpoints[1] - midpoints[3]).Norm());
    hs.push_back((midpoints[2] - midpoints[4]).Norm());
    min = max = hs[0];
    
    for(int i=1;i<3;i++){
        if(hs[i] < min)
            min = hs[i];
        if(hs[i] > max)
            max = hs[i];
    }
    if(min==0){
        cout << "oops: min Hex AR is 0\n";
        return 0;
    }
    return min/max;
}

double Hexahedron::getEdgeAspectRatio(vector<Point3D> &points) {
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::min();
    for (unsigned int i=0; i<12; i++) {
        vector<unsigned int> e = getEdge(i);
        Point3D p1 = points[e[0]];
        Point3D p2 = points[e[1]];
        double dis = (p1-p2).Norm();
        if (dis<min) {
            min = dis;
        }
        if (dis>max) {
            max = dis;
        }
    }
    return min/max;
}


vector<unsigned int> Hexahedron::getEdge(const unsigned int &eIdx) {
    vector<unsigned int> edge(2,0);
    switch (eIdx) {
        case 0:
            edge[0] = 0;
            edge[1] = 3;
            return edge;
        case 1:
            edge[0] = 3;
            edge[1] = 2;
            return edge;
        case 2:
            edge[0] = 2;
            edge[1] = 1;
            return edge;
        case 3:
            edge[0] = 1;
            edge[1] = 0;
            return edge;
        case 4:
            edge[0] = 4;
            edge[1] = 5;
            return edge;
        case 5:
            edge[0] = 5;
            edge[1] = 6;
            return edge;
        case 6:
            edge[0] = 6;
            edge[1] = 7;
            return edge;
        case 7:
            edge[0] = 7;
            edge[1] = 4;
            return edge;
        case 8:
            edge[0] = 0;
            edge[1] = 4;
            return edge;
        case 9:
            edge[0] = 1;
            edge[1] = 5;
            return edge;
        case 10:
            edge[0] = 2;
            edge[1] = 6;
            return edge;
        case 11:
            edge[0] = 3;
            edge[1] = 7;
            return edge;
        default:
            return edge;
    }
}


vector<unsigned int> Hexahedron::getFace(const unsigned int &fIdx) {
    vector<unsigned int> face(4,0);
    switch (fIdx) {
        case 0:
            face[0] = 0;
            face[1] = 3;
            face[2] = 2;
            face[3] = 1;
            return face;
        case 1:
            face[0] = 0;
            face[1] = 1;
            face[2] = 5;
            face[3] = 4;
            return face;
        case 2:
            face[0] = 1;
            face[1] = 2;
            face[2] = 6;
            face[3] = 5;
            return face;
        case 3:
            face[0] = 2;
            face[1] = 3;
            face[2] = 7;
            face[3] = 6;
            return face;
        case 4:
            face[0] = 3;
            face[1] = 0;
            face[2] = 4;
            face[3] = 7;
            return face;
        case 5:
            face[0] = 4;
            face[1] = 5;
            face[2] = 6;
            face[3] = 7;
            return face;
        default:
            return face;
    }
}

vector<vector<unsigned int> > Hexahedron::getTetras() {
    
    vector<vector<unsigned int> > tets;
    tets.reserve(8);
    
    vector<unsigned int> t0(4,0);
    t0[0]=points[0];
    t0[1]=points[1];
    t0[2]=points[3];
    t0[3]=points[4];
    tets.push_back(t0);
    
    vector<unsigned int> t1(4,0);
    t1[0]=points[1];
    t1[1]=points[2];
    t1[2]=points[0];
    t1[3]=points[5];
    tets.push_back(t1);
    
    vector<unsigned int> t2(4,0);
    t2[0]=points[2];
    t2[1]=points[3];
    t2[2]=points[1];
    t2[3]=points[6];
    tets.push_back(t2);
    
    vector<unsigned int> t3(4,0);
    t3[0]=points[3];
    t3[1]=points[0];
    t3[2]=points[2];
    t3[3]=points[7];
    tets.push_back(t3);
    
    vector<unsigned int> t4(4,0);
    t4[0]=points[4];
    t4[1]=points[7];
    t4[2]=points[5];
    t4[3]=points[0];
    tets.push_back(t4);
    
    vector<unsigned int> t5(4,0);
    t5[0]=points[5];
    t5[1]=points[4];
    t5[2]=points[6];
    t5[3]=points[1];
    tets.push_back(t5);
    
    vector<unsigned int> t6(4,0);
    t6[0]=points[6];
    t6[1]=points[5];
    t6[2]=points[7];
    t6[3]=points[2];
    tets.push_back(t6);
    
    vector<unsigned int> t7(4,0);
    t7[0]=points[7];
    t7[1]=points[6];
    t7[2]=points[4];
    t7[3]=points[3];
    tets.push_back(t7);
    
    return tets;
}

vector<unsigned int> Hexahedron::getTetra(int index){
    
    vector<unsigned int> t(4,0);
    
    if(index == 0){
        t[0]=points[0];
        t[1]=points[1];
        t[2]=points[3];
        t[3]=points[4];
    }
    else if(index==1){
        t[0]=points[1];
        t[1]=points[2];
        t[2]=points[0];
        t[3]=points[5];
    }
    else if(index==2){
        t[0]=points[2];
        t[1]=points[3];
        t[2]=points[1];
        t[3]=points[6];
    }
    else if(index==3){
        t[0]=points[3];
        t[1]=points[0];
        t[2]=points[2];
        t[3]=points[7];
    }
    else if(index==4){
        t[0]=points[4];
        t[1]=points[7];
        t[2]=points[5];
        t[3]=points[0];
    }
    else if(index==5){
        t[0]=points[5];
        t[1]=points[4];
        t[2]=points[6];
        t[3]=points[1];
    }
    else if(index==6){
        t[0]=points[6];
        t[1]=points[5];
        t[2]=points[7];
        t[3]=points[2];
    }
    else if(index==7){
        t[0]=points[7];
        t[1]=points[6];
        t[2]=points[4];
        t[3]=points[3];
    }
    
    return t;
}
