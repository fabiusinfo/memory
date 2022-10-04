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


#ifndef Main_h
#define Main_h 1

//memorist
#include <string.h>

#include <limits.h>
#include <memory>
#include <utility>
#include <thread> 

#include <string>
#include <cctype>
#include <iostream>
#include "Point3D.h"
#include "Hexahedron.h"
#include "Prism.h"
#include "Pyramid.h"
#include "Tetrahedron.h"
#include "Quality.h"
#include <math.h>

using Clobscode::Point3D;
using namespace std;

void emsg(){
	cout << "use: ./jacobian -option input.m3d\n";
	cout << "options:\n";
	cout << "  -s : Scaled Jacobian statistics\n";
	cout << "  -e : Element Normalized Scaled Jacobiab statistics\n";
	cout << "  -j : List Jens of each Element Jens [Js]\n";
    cout << "  -a : List Aspect Ratio for each element\n";
    cout << "  -l : List Jens of each node for each element\n";
}

bool readData(string name, vector<Point3D> &points, vector<Element *> &elements){
    
    
    char word [256];
    int pcant=0, ecant=0;
    double x=0,y=0,z=0;
    unsigned int line = 0;
    
    bool mvm = false;
    
    FILE *file = fopen(name.c_str(),"r");
    
    if (file==NULL) {
        std::cout << "File " << name << " doesn't exist\n";
        return false;
    }
    
    //read number of nodes
    while(true){
        if(std::fscanf(file,"%s",word)==EOF){
            fclose(file);
            return false;
        }
        if(!strcmp(word,"ARRAY1<STRING>]\0"))
            break;
        if (!strcmp(word,"MIXED\0")) {
            mvm = true;
            break;
        }
    }
    
    std::fscanf(file,"%i",&pcant);
    
    if(pcant<=0){
        fclose(file);
        return false;
    }
    
    if (mvm) {
        std::fscanf(file,"%i",&ecant);
        
        if(ecant<=0){
            fclose(file);
            return false;
        }
    }

    //read each node
    points.reserve(pcant);
    
    for( int i=0;i<pcant;i++){
        if (!mvm) {
            std::fscanf(file,"%s",word);
        }
        std::fscanf(file,"%s",word);
        x=atof(word);
        std::fscanf(file,"%s",word);
        y=atof(word);
        std::fscanf(file,"%s",word);
        z=atof(word);
        Point3D p (x,y,z);
        points.push_back(p);
    }
    
    //read number of elements
    if (!mvm) {
        while(1){
            if(std::fscanf(file,"%s",word) == EOF){
                std::cout << "didn't find elements\n";
                fclose(file);
                return false;
            }
            
            if(!strcmp(word,"ARRAY1<STRING>]\0")){
                //std::fscanf(file,"%s",word);
                std::fscanf(file,"%i",&ecant);
                break;
            }
        }
        
        if(ecant<=0){
            fclose(file);
            return false;
        }
    }
    
    elements.reserve(ecant);
    
    //read each element
    for( int i=0;i<ecant;i++){
        std::fscanf(file,"%s",word);
        unsigned int nodenumber = 0;
        
        if (!strcmp(word,"H\0")){
            vector<int> element(8,0);
            for(unsigned int j=0;j<8;j++){
                fscanf(file,"%i",&element[j]);
            }
            elements.push_back(new Hexahedron(element));
        }
        else if (!strcmp(word,"R\0")){
            vector<int> element(6,0);
            for(unsigned int j=0;j<6;j++){
                fscanf(file,"%i",&element[j]);
            }
            elements.push_back(new Prism(element));
        }
        else if (!strcmp(word,"P\0")){
            vector<int> element(5,0);
            for(unsigned int j=0;j<5;j++){
                fscanf(file,"%i",&element[j]);
            }
            elements.push_back(new Pyramid(element));
        }
        else if (!strcmp(word,"T\0")){
            vector<int> element(4,0);
            for(unsigned int j=0;j<4;j++){
                fscanf(file,"%i",&element[j]);
            }
            elements.push_back(new Tetrahedron(element));
        }
        else {
            cerr << "unknown element type at element " << i << "\n";
            fclose(file);
            return false;
        }
        
        //read some unnecessary integers
        if (!mvm) {
            for(unsigned int j=0;j<3;j++)
                std::fscanf(file,"%s",word);
        }
    }
    fclose(file);
    
    return true;
}

int main(int argc,char** argv){
	
	vector<Element *> elements;
	vector<Point3D> points;

    Quality quality;
	
	if(argc!=3){
		emsg();
		return 1;
	}
	
    if(!readData(argv[2],points,elements)) {
        return 1;
    }
	
	if(argv[1][1] == 's'){
        quality.execute_JS(points,elements);
	}
	else if(argv[1][1] == 'e'){
		quality.execute_JENS(points,elements);
	}
    else if(argv[1][1] == 'j'){
        quality.execute_all(points,elements);
    }
    else if(argv[1][1] == 'a'){
        quality.execute_allAR(points,elements);
    }
    else if(argv[1][1] == 'l'){
        quality.execute_allJENS(points,elements);
    }
    
	for(int i=0;i<elements.size();i++){
		delete elements[i];
	}
	
	return 0;
}

#endif

