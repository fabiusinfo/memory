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

#ifndef Quality_h
#define Quality_h 1

#include "Hexahedron.h"
#include "Prism.h"
#include "Pyramid.h"
#include "Tetrahedron.h"
#include "Point3D.h"

#include <map>
#include <tuple>

using namespace std;

class Quality {
    
public:
    Quality();
    
    virtual ~Quality();
	
	/*virtual bool repair(vector<Point3D> &p, vector<Element *> &e, 
					 	char &metric, unsigned int &max_iter, double &threshold);*/
						
	virtual bool repair(vector<Point3D> &p, vector<Element *> &e, 
						char &metric, unsigned int &max_iter, const double &threshold);
						
	/*virtual bool repairOne(vector<Point3D> &p, vector<Element *> &e,
						   unsigned int &eidx, unsigned int &nodelocalidx,
						   char &metric, unsigned int &max_iter,
						   double &current_qua, const double &threshold);*/
    
	virtual bool repairOne(vector<Point3D> &p, vector<Element *> &e,
					   	   unsigned int &nodeidx, char &metric, 
					   	   unsigned int &max_iter, const double &threshold);
	
    virtual void execute_JS(vector<Point3D> &points, vector<Element *> &elements);
    
    virtual void execute_JENS(vector<Point3D> &points, vector<Element *> &elements);
    
    virtual void execute_all(vector<Point3D> &points, vector<Element *> &elements);
    
    virtual void execute_allAR(vector<Point3D> &points, vector<Element *> &elements);
	
protected:
	
	virtual void linkElementsToNodes(vector<Element *> &e);
	map<unsigned int, list<tuple<unsigned int, unsigned int> > > nodeElements;
    
};

#endif

