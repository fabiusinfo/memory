/**************************************************************************/
/*                                                                        */
/*                  Jens Quality Improvement                              */
/*                                                                        */
/**************************************************************************/
/* Allows to improve mesh quality reading a file from .m3d format         */
/* Written by Claudio Lobos (clobos@inf.utfsm.cl) 2021					  */
/* UNIVERSIDAD TECNICA FEDERICO SANTA MARIA								  */
/**************************************************************************/

/*
 <Quality Improvement JENS: this program improves the quality by using the
 Element Normalized Scaled Jacobian (Jens). It also allows to use other
 quality metrics for comparison>
 
 Copyright (C) <2021>  <Claudio Lobos>
 
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

#include <string>
#include <cctype>
#include <iostream>
#include "Point3D.h"
#include "Hexahedron.h"
#include "Prism.h"
#include "Pyramid.h"
#include "Tetrahedron.h"
#include "Quality.h"
#include "MetricJens.h"
#include "TriMesh.h"

#include "MetricJR.h"
#include <math.h>

//memorist
#include "FaceContainer.h"
#include "Element.h"
#include "Services.h"

using Clobscode::Point3D;
using namespace std;


//-------------------------------------------------------------------
//-------------------------------------------------------------------
void emsg(){
	cout << "use: ./jens_repair input inputSF metric max_iter threshold output\n";
	cout << "where:\n";
	cout << "  input: is a volume mesh in m3d format.\n";
	cout << "  metric: is the function to be use for quality optimization.\n";
	cout << "          can be \"e\" for Jens or \"r\" for Jacobian Ratio\n";
	cout << "  max_iter: number of iteration at which the algorithm will halt.\n";
    cout << "  threshold: desired quality value to be achived (<=1)\n";
	cout << "  output: file name of the resulting mesh (m3d).\n";
	cout << "  inputSF: is a surface mesh in mdl format.\n";
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
//This method allows to read from a M3d or MVM format.
bool readM3d(string name, vector<Point3D> &points, vector<Element *> &elements){
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

//-------------------------------------------------------------------
//-------------------------------------------------------------------
bool writeM3dMesh(std::string name, vector<Point3D> &points, vector<Element *> &elements){
	
	if (elements.empty()) {
		std::cout << "no output elements\n";
		return false;
	}
	
	string vol_name = name+".m3d";
	
	//write the volume mesh
	FILE *f = fopen(vol_name.c_str(),"wt");
	
	unsigned int n = points.size();
	
	fprintf(f,"%s\n","[Nodes, ARRAY1<STRING>]");
	fprintf(f,"%i\n\n",n);
	
	//write points
	for(unsigned int i=0;i<n;i++){			
		fprintf(f,"1 %+1.8E",points[i][0]);
		fprintf(f," %+1.8E",points[i][1]);
		fprintf(f," %+1.8E\n",points[i][2]);
	}
	
	n = elements.size();
	
	fprintf(f,"\n%s\n","[Elements, ARRAY1<STRING>]");
	fprintf(f,"%u\n\n",n);
	
	//get all the elements in a std::vector
	for (unsigned int i=0; i<n; i++) {
		std::vector<unsigned int> epts = elements[i]->getPoints();
		unsigned int np = epts.size();
		if (np == 4) {
			fprintf(f,"T");
		}
		else if (np == 5){
			fprintf(f,"P");
		}
		else if (np == 6){
			fprintf(f,"R");
		}
		else if (np == 8){
			fprintf(f,"H");
		}
		
		for (unsigned int j= 0; j<np; j++) {
			fprintf(f," %u", epts.at(j));
		}
		
		fprintf(f," 1000.0 0.45 1.0\n");
	}
	fclose(f);
	
	return true;
}

//memorist

bool readData(char* name, const unsigned int &type, vector<Point3D> &points, vector<Element *> &elements,FaceContainer &all_faces){
	switch (type) {
        case 0:
            readM3d(name, points, elements);
            break;
        //case 1:
          //  readMixed(name);
            //break;
        //case 2:
          //  readVtk(name);
           // break;
        default:
            return false;
    }
    
	if ((int)points.size()<3) {
		cout << "cannot display a mesh with less than 3 nodes\n";
		return false;
	}
	
	if ((int)elements.size()<1) {
		cout << "mesh has no elements: nothing to display\n";
		return false;
	}
	
	//save faces with neigbhoring information
	for(unsigned int i=0; i<elements.size(); i++){
		//An EnhancedElement is an Element with optimizations for
		//removing and visualizing.
		elements[i]->computeBbox(points);
		
		//get the element face index points and initialize it
		for(int j=0;j<elements[i]->numberOfFaces();j++){
            
			vector<unsigned int> fpts = elements[i]->getFacePoints(j);
            		Face face(fpts);
			int fid = all_faces.addFace(face);
			all_faces.getFace(fid).addElement(i);
			//add the face to the EnhancedElement
			elements[i]->addFace(fid);
		}
	}
	
	return true;

}
//memorist

void headGeom(ofstream &output, int np, int nf){
	
	output << "OFF\n";
    output << np << " " << nf << " 0\n\n";
}

void dataGeom(ofstream &output, list<Point3D> &tpoints, list<Face> &tfaces){
	list<Point3D>::iterator iterp;
	list<Face>::iterator iterf;
	//print each point
	for(iterp=tpoints.begin();iterp!=tpoints.end();iterp++){
		Point3D p = *iterp;
		output << p[0] << " ";
		output << p[1] << " ";
		output << p[2] << "\n";
	}
	
	output << endl;
	
	//print each face
	for(iterf=tfaces.begin();iterf!=tfaces.end();iterf++){
		output << *iterf << " ";
		output << (*iterf).getColor() << endl;
	}
}
bool newTmpFile(string &stmp,ofstream &output){
    char tmp [16];
    int fd;
	
    strcpy(tmp,"geomviewXXXXXX\0");
    
    fd=mkstemp(tmp);
    
    if(fd==-1){
		cout << "  Error: Can't create a tmp file. Try saving";
		cout << " before viewing.\n";
		return false;
    }
    
    stmp+=tmp;
    output.open(tmp);
    return true;
}

//save in tfaces only the faces that belong to one element from the given
//FaceContainer. Then it only save in tpoints the points used by the
//selected faces.
void clean(list<Point3D> &tpoints, vector<Point3D> &points, list<Face> &tfaces,
					FaceContainer &tfc){
	
	vector<int> ptsidx (points.size(),-1);
	int usedpts = 0;
	for(int i=0;i<tfc.length();i++){
		if(tfc.getFace(i).numberOfElements()!=1)
			continue;
		vector<unsigned int> fpts = tfc.getFace(i).getPoints();
		for(int j=0;j<fpts.size();j++){
			if(ptsidx[fpts[j]]==-1){
				tpoints.push_back(points[fpts[j]]);
				ptsidx[fpts[j]]=usedpts;
				fpts[j]=usedpts++;
			}
			else{
				fpts[j]=ptsidx[fpts[j]];
			}
		}
		Face f(fpts);
		f.setColor(tfc.getFace(i).getColor());
		tfaces.push_back(f);
	}
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

void cleanUnusedNodes(list<Point3D> &tpoints,vector<Point3D> &points,
							   list<Face> &tfaces,
							   FaceContainer &tfc){
	
	vector<int> ptsidx (points.size(),-1);
	int usedpts = 0;
	for(int i=0;i<tfc.length();i++){
		vector<unsigned int> fpts = tfc.getFace(i).getPoints();
		for(int j=0;j<fpts.size();j++){
			if(ptsidx[fpts[j]]==-1){
				tpoints.push_back(points[fpts[j]]);
				ptsidx[fpts[j]]=usedpts;
				fpts[j]=usedpts++;
			}
			else{
				fpts[j]=ptsidx[fpts[j]];
			}
		}
		Face f(fpts);
		f.setColor(tfc.getFace(i).getColor());
		tfaces.push_back(f);
	}
}

//--
//-------------------------------------------------------------------
//-------------------------------------------------------------------
int main(int argc,char** argv){
	
	vector<Element *> elements;
	vector<Point3D> points;
	
	FaceContainer all_faces;
	const unsigned int &type = 0;
	
	//memorist
	string file;
	ofstream fgeom;
	list<Point3D> tmppoints;
	list<Face> tmpfaces;
	list<Point3D> tmppoints2;
	list<Face> tmpfaces2;
	vector <Element *>tmpElements;
	list<unsigned int> inv_nodesS;
	
	//mdl
	vector<Point3D> tri_pts;
	vector<vector<unsigned int> > tri_allfaces;

	vector<Clobscode::TriMesh> clobs_inputs;
	
	//--
    Clobscode::Services services;
    Quality quality;
	if(argc!=7){
		emsg();
		return 1;
	}
	
	//argv[1][1] should contain input file type (only m3d for now).
	
	//memorist
    //read m3d
    if(!readData(argv[1], type, points, elements,all_faces)){
    	cout<<"aca cagó"<<endl;
    	return 1;
    }
    //read mdl
    if (!services.ReadMdlMesh(argv[6], tri_pts, tri_allfaces, clobs_inputs)){
    	return 1;
    	}
    	
    
    
    FaceContainer surface_fc;
    //imprimimos las caras de la superficie
    int cont=0;
    bool element=false;

    for (unsigned int i=0; i<elements.size(); i++){
    	//if (elements[i]->surfaceElement==true)
    		elements[i]->ESChecker(all_faces, surface_fc);
    	}

	cout<<"this is the total of faces of surfaces elements"<<endl;
	cout<<cont<<endl;
	/*
	if(!newTmpFile(file,fgeom)){
			cout << "  Error: can't generate a tmp file for visualization\n";
			return false;
		}
	*/

	//cleanUnusedNodes(tmppoints,points, tmpfaces,surface_fc);
		
	//headGeom(fgeom,tmppoints.size(),tmpfaces.size());
	//dataGeom(fgeom,tmppoints,tmpfaces);
        
	//fgeom.close();
	//--
	char metricType = argv[2][1];
	unsigned int max_iter = atoi(argv[3]);
	double threshold = atof(argv[4]);
	
	Metric *metric;
	map<int, list<int>> neighborsmap;
	services.NeighborgKnowlegde(elements, points, neighborsmap);
	map<int,list<int>>::iterator iter;
	list<int>::iterator NeighborNode;

	/*for (iter=neighborsmap.begin(); iter !=neighborsmap.end(); iter++ ){

		cout<<"largo de la lista de vecinos del nodo "<<iter->first<<" es: "<<iter->second.size()<<endl;
	
	}*/
	
	if (metricType == 'e') {
		//metric = new MetricJens(points);
		metric = new MetricJens();
	} else if (metricType == 'r') {
		metric = new MetricJR();
	}
	quality.repair(points, inv_nodesS, elements,metric,max_iter,threshold, all_faces, neighborsmap);
	
	/*
	//projection
	for (auto node: inv_nodesS){
		Clobscode::Point3D p = points[node];
		points[node] = Clobscode::TriMesh(tri_pts, tri_allfaces).getProjection(p);
	}
	*/
	
	writeM3dMesh(argv[5],points,elements);
	
	/*if(argv[1][1] == 's'){
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
    }*/
    
	for(int i=0;i<elements.size();i++){
		delete elements[i];
	}
	
	return 0;
}

#endif

