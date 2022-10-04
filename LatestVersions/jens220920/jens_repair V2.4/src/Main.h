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
#include "../read_write/ReadAnsys.h"
#include "../read_write/ReadM3d.h"
#include "../read_write/WriteAnsys.h"
#include "../read_write/WriteM3d.h"
#include "EnhancedPoint.h"
#include "JacHexahedra.h"
#include "JacPrism.h"
#include "JacTetrahedra.h"
#include "JacPyramid.h"
#include "JacDummy.h"
#include "Invalid.h"
#include <math.h>

//memorist
protected:
	
	char op;
	FaceContainer all_faces;
	
	vector<Point3D> points;
	vector<Element *> elements;
	Point3D min, max;
	
//--

#endif

