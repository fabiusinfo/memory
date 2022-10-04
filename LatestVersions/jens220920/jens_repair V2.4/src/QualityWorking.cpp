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

#include "Quality.h"

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
Quality::Quality(){
    
}


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
Quality::~Quality(){
    
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void Quality::linkElementsToNodes(vector<Element *> &e) {
	//create a map to link nodes with their elements for fast access:
	for (unsigned int i=0;i<e.size();i++){
		vector<unsigned int> enodes = e[i]->getPoints();
		for (unsigned int j=0;j<enodes.size();j++) {
			auto found = nodeElements.find(enodes[j]);
	        if (found != nodeElements.end()) {
	            found->second.push_back(std::make_tuple(i,j));
	        }
	        else {
				list<tuple<unsigned int, unsigned int> > nelem;
				nelem.push_back(std::make_tuple(i,j));
	            nodeElements.emplace(enodes[j], nelem);
	        }
		}
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
bool Quality::repair(vector<Point3D> &p, vector<Element *> &e, 
					 char &metric, unsigned int &max_iter, const double &threshold){
		
	//This method can be called several times with different values for 
	//threshold. Only the fisrt time we want to link elements to nodes.
	if (nodeElements.empty()) {
		linkElementsToNodes(e);
	}
	
	//Make a list with all unacceptable nodes
	list<unsigned int> inv_nodes;
	for (unsigned int i=0;i<e.size();i++){
		vector<double> node_jens = e[i]->getJENS(p);
		for (unsigned int j=0;j<node_jens.size();j++) {
			if (node_jens[j] < threshold) {
				inv_nodes.push_back(e[i]->getPoints()[j]);
			}
		}
	}
	//consider them only once:
	inv_nodes.sort();
	inv_nodes.unique();
	
	//Repair them.
	bool valid = true;
	for (auto node: inv_nodes){
		if (!repairOne(p,e,node,metric,max_iter,0)) {
			valid = false;
		}
	}
	cout << " >Number of unacceptable nodes: "<< inv_nodes.size() << endl;
	if (!valid) {
		cout << "  Some nodes could not be repaired to achieve the threshold.\n";
		cout << "  The process finished. Resulting mesh written to output.\n";
		return false;
	}
	else {
		cout << "  All elements achieved the threshold.\n";
	}
	
	
	//Detect elements with node quality below the threshold
	bool quality = true;
	unsigned int acceptable = 0;
	for (unsigned int i=0;i<e.size();i++){
		vector<double> node_jens = e[i]->getJENS(p);
		double worst = 1;
		unsigned int widx = 0, last_time = UINT_MAX;
		//find the worst node within this element
		for (unsigned int j=0;j<node_jens.size();j++) {
			double current = node_jens[j];
			if (current < threshold && current < worst) {
				worst = current;
				widx = j;
			}
		}
		//cout << worst << " " << threshold << endl;
		while (worst<threshold) {
			if (widx==last_time) {
				break;
			}
			if (worst < threshold) {
				acceptable++;
				if (!repairOne(p,e,e[i]->getPoints()[widx],metric,max_iter,threshold)) {
					quality = false;
				}
				last_time = widx;
			}
			worst = 2;
			for (unsigned int j=0;j<node_jens.size();j++) {
				double current = node_jens[j];
				if (current < worst) {
					worst = current;
					widx = j;
				}
			}
		}
	}
	cout << " >Number of nodes below target threshold: "<< acceptable << endl;
	if (!quality) {
		cout << "  Some nodes did not reach target threshold\n";
		cout << "  due to the number of max_iterations was too small.\n";
		cout << "  The process finished. Resulting mesh written to output\n";
		return false;
	}
	else {
		cout << "  All elements reached target quality.\n";
	}
	
	return true;
}

bool Quality::repairOne(vector<Point3D> &p, vector<Element *> &e,
						unsigned int &nodeidx, char &metric, 
						unsigned int &max_iter, const double &threshold) {
	
	//Compute current quality considering all elements attached to this node
	//that don't achive the threshold.
	double current_qua = 0;
	unsigned int num_inv = 0;
	list<tuple<unsigned int,unsigned int> > elemlist = nodeElements[nodeidx];
	for (auto elem: elemlist) {
		//compute the quality within this element
		double qua = e[std::get<0>(elem)]->getJENS(p)[std::get<1>(elem)];
		if (qua<threshold) {
			current_qua += qua;
			num_inv++;
			//cout << "num inv: "<< num_inv << " (" << std::get<0>(elem) << ",";
			//cout << std::get<1>(elem) << ")\n";
		}
	}
	
	if (num_inv>0) {
		current_qua /= num_inv;
	}
	else {
		return true;
	}
	
	
	
	//compute gradient for this node:
	double epsilon = 0.001, step = 1, new_qua = 0;
	unsigned int iter = 0;
	
	cout << "   >for node " << nodeidx;
	cout << " initial pos " << p[nodeidx] << " (" << current_qua << ")\n";
	
	while (current_qua<threshold && iter<max_iter) {
		//Point3D original = p[node_idx];

		num_inv = 0;
		
		//compute delta in X:
		p[nodeidx][0] += epsilon;
		new_qua = 0;
		for (auto elem: elemlist) {
			//compute the quality within this element
			double qua = e[std::get<0>(elem)]->getJENS(p)[std::get<1>(elem)];
			if (qua<threshold) {
				new_qua += qua;
				num_inv++;
			}
		}
		new_qua /= num_inv;
		
		double deltaX = (new_qua - current_qua)/epsilon; 
		p[nodeidx][0] -= epsilon;
	
		//compute delta in Y:
		p[nodeidx][1] += epsilon;
		new_qua = 0;
		for (auto elem: elemlist) {
			//compute the quality within this element
			double qua = e[std::get<0>(elem)]->getJENS(p)[std::get<1>(elem)];
			if (qua<threshold) {
				new_qua += qua;
			}
		}
		new_qua /= num_inv;
		
		double deltaY = (new_qua - current_qua)/epsilon; 
		p[nodeidx][1] -= epsilon;
	
		//compute delta in Z:
		p[nodeidx][2] += epsilon;
		new_qua = 0;
		for (auto elem: elemlist) {
			//compute the quality within this element
			double qua = e[std::get<0>(elem)]->getJENS(p)[std::get<1>(elem)];
			if (qua<threshold) {
				new_qua += qua;
			}
		}
		new_qua /= num_inv;
		
		double deltaZ = (new_qua - current_qua)/epsilon; 
		p[nodeidx][2] -= epsilon;
	
		Point3D direction(deltaX,deltaY,deltaZ);
		direction.normalize();
		
		p[nodeidx] += direction*step;
		
		current_qua = 0;
		num_inv = 0;
		double one_qua = 0;
		for (auto elem: elemlist) {
			//compute the quality within this element
			double qua = e[std::get<0>(elem)]->getJENS(p)[std::get<1>(elem)];
			if (qua<threshold) {
				current_qua += qua;
				num_inv++;
			}
			else {
				one_qua = qua;
			}
		}
		
		iter++;
		
		if (num_inv>0) {
			current_qua /= num_inv;
		}
		else {
			current_qua = one_qua;
			break;
		}
		
		
		/*if (iter<10) {
			cout << "     0";
		}
		else {
			cout << "     ";
		}
			
		
		cout << iter << " pos " << p[nodeidx] << " (" << current_qua << ")\n";*/
		
	}
	
	cout << "    final pos " << p[nodeidx] << endl;
	
	cout << "    for node " << nodeidx << " final quality is " << current_qua;
	cout << " in " << iter << " iterations\n";
	
	if (current_qua<threshold) {
		cout << "Warning: for node " << nodeidx << " max iteration reached without";
		cout << " achieving targer threshold (" << threshold << ")\n";
		return false;
	}
	
	return true;
}


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void Quality::execute_JS(vector<Point3D> &p, vector<Element *> &e){
    
    if (e.empty()){
        cout << "no elements\n";
        return;
    }
    vector<int> histo (20,0);
    double worst = 2, total = 0;
    
    vector<double> min_vals_type (4,2), max_vals_type (4,-2), ave_vals_type (4,0);
    vector<unsigned int> ele_quantity (4,0);
    
    unsigned int type = 0;
    
    for (int i=0; i<(int)e.size(); i++) {
        double qua = e[i]->getElementJS(p);
        total += qua;
        bool print = false;
        if(worst > qua)
            worst = qua;
        if(qua<0){
            histo[0]++;
            print = true;
        }
        else if(qua<0.033333){
            histo[1]++;
            print = true;
        }
        else if(qua<0.05)
            histo[2]++;
        else if(qua<0.1)
            histo[3]++;
        else if(qua<0.15)
            histo[4]++;
        else if(qua<0.2)
            histo[5]++;
        else if(qua<0.25)
            histo[6]++;
        else if(qua<0.3)
            histo[7]++;
        else if(qua<0.35)
            histo[8]++;
        else if(qua<0.4)
            histo[9]++;
        else if(qua<0.45)
            histo[10]++;
        else if(qua<0.5)
            histo[11]++;
        else if(qua<0.55)
            histo[12]++;
        else if(qua<0.6)
            histo[13]++;
        else if(qua<0.65)
            histo[14]++;
        else if(qua<0.7)
            histo[15]++;
        else if(qua<0.85)
            histo[16]++;
        else if(qua<0.9)
            histo[17]++;
        else if(qua<0.95)
            histo[18]++;
        else if(qua<=1)
            histo[19]++;
        
        
        /*else{
            cerr << "Quality quality value at Quality::surfaceQuality: ";
            cerr << qua << endl;
        }*/
        
        if (print) {
            cout << i << " " << *(e[i]) << " 1000.0 0.45 1.0\n";
        }
        
        //find the type
        if(e[i]->getCharType()=='H') {
            type=0;
        }
        else if(e[i]->getCharType()=='R') {
            type=1;
        }
        else if(e[i]->getCharType()=='P') {
            type=2;
        }
        else if(e[i]->getCharType()=='T') {
            type=3;
        }
        
        //update min and max per element type.
        if(min_vals_type[type] > qua){
            min_vals_type[type] = qua;
        }
        if(qua > max_vals_type[type]){
            max_vals_type[type] = qua;
        }
        
        ave_vals_type[type] += qua;
        ele_quantity[type]++;
        
    }
    cout << "negative: " << histo[0] << endl;
    cout << "<0.0333 : " << histo[1] << endl;
    float step = 0.05;
    for(int i=2;i<19;i++) {
        cout << "<" << step << "\t:" << histo[i] << endl;
        step+=0.05;
    }
    cout << "<1 " << histo[19] << endl;
    cout << "total: " << e.size() << endl;
    cout << "worst quality " << worst << endl;
    cout << "average quality " << total/e.size() << endl;
    
    cout << "\ntikz format\n";
    cout << "(-1," << histo[0] << ")\n";
    cout << "(0.03," << histo[1] << ")\n";
    step = 0.05;
    for(int i=2;i<19;i++){
        cout << "(" << step << "," << histo[i] << ")\n";
        step+=0.05;
    }
    cout << "(1," << histo[19] << ")\n";
    
    //print range of quality per element type
    cout << " > Quality per element type: Hex, Pri, Pyr, Tet respectively:\n";
    for (unsigned int i=0; i<min_vals_type.size(); i++) {
        if (ele_quantity[i]>=2) {
            cout << "[ " << min_vals_type[i] << "," << max_vals_type[i] << "] average: ";
            cout << ave_vals_type[i]/ele_quantity[i] << " (" << ele_quantity[i] << ")\n";
        }
        if (ele_quantity[i]==1) {
            cout << "[ " << ave_vals_type[i] << "," << ave_vals_type[i] << "] average: ";
            cout << ave_vals_type[i] << " (" << ave_vals_type[i] << ")\n";
        }
        else {
            cout << "no elements of this type\n";
        }
    }
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void Quality::execute_JENS(vector<Point3D> &p, vector<Element *> &e){
    
    if (e.empty()){
        cout << "no elements\n";
        return;
    }
    vector<int> histo (20,0);
    double worst = 2, total = 0;
    
    vector<double> min_vals_type (4,2), max_vals_type (4,-2), ave_vals_type (4,0);
    vector<unsigned int> ele_quantity (4,0);
    
    unsigned int type = 0;
    
    for (int i=0; i<(int)e.size(); i++) {
        double qua = e[i]->getElementJENS(p);
        total += qua;
        bool print = false;
        if(worst > qua)
            worst = qua;
        if(qua<0){
            histo[0]++;
            print = true;
        }
        else if(qua<0.033333){
            histo[1]++;
            print = true;
        }
        else if(qua<0.05)
            histo[2]++;
        else if(qua<0.1)
            histo[3]++;
        else if(qua<0.15)
            histo[4]++;
        else if(qua<0.2)
            histo[5]++;
        else if(qua<0.25)
            histo[6]++;
        else if(qua<0.3)
            histo[7]++;
        else if(qua<0.35)
            histo[8]++;
        else if(qua<0.4)
            histo[9]++;
        else if(qua<0.45)
            histo[10]++;
        else if(qua<0.5)
            histo[11]++;
        else if(qua<0.55)
            histo[12]++;
        else if(qua<0.6)
            histo[13]++;
        else if(qua<0.65)
            histo[14]++;
        else if(qua<0.7)
            histo[15]++;
        else if(qua<0.85)
            histo[16]++;
        else if(qua<0.9)
            histo[17]++;
        else if(qua<0.95)
            histo[18]++;
        else if(qua<=1)
            histo[19]++;
        
        /*else{
            cerr << "Quality quality value at Quality::surfaceQuality: ";
            cerr << qua << endl;
        }*/
        
        if (print) {
            cout << i << "\n";// << " " << *(e[i]) << " 1000.0 0.45 1.0\n";
        }
        
        //find the type
        if(e[i]->getCharType()=='H') {
            type=0;
        }
        else if(e[i]->getCharType()=='R') {
            type=1;
        }
        else if(e[i]->getCharType()=='P') {
            type=2;
        }
        else if(e[i]->getCharType()=='T') {
            type=3;
        }
        
        //update min and max per element type.
        if(min_vals_type[type] > qua){
            min_vals_type[type] = qua;
        }
        if(qua > max_vals_type[type]){
            max_vals_type[type] = qua;
        }
        
        ave_vals_type[type] += qua;
        ele_quantity[type]++;
        
    }
    
    cout << "negative: " << histo[0] << endl;
    cout << "<0.0333 : " << histo[1] << endl;
    float step = 0.05;
    for(int i=2;i<19;i++) {
        cout << "<" << step << "\t:" << histo[i] << endl;
        step+=0.05;
    }
    cout << "<1 " << histo[19] << endl;
    cout << "total: " << e.size() << endl;
    cout << "worst quality " << worst << endl;
    cout << "average quality " << total/e.size() << endl;
    
    cout << "\ntikz format\n";
    cout << "(-1," << histo[0] << ")\n";
    cout << "(0.03," << histo[1] << ")\n";
    step = 0.05;
    for(int i=2;i<19;i++){
        cout << "(" << step << "," << histo[i] << ")\n";
        step+=0.05;
    }
    cout << "(1," << histo[19] << ")\n";

    
    //print range of quality per element type
    cout << " > Quality per element type: Hex, Pri, Pyr, Tet respectively:\n";
    for (unsigned int i=0; i<min_vals_type.size(); i++) {
        if (ele_quantity[i]>=2) {
            cout << "[ " << min_vals_type[i] << "," << max_vals_type[i] << "] average: ";
            cout << ave_vals_type[i]/ele_quantity[i] << " (" << ele_quantity[i] << ")\n";
        }
        if (ele_quantity[i]==1) {
            cout << "[ " << ave_vals_type[i] << "," << ave_vals_type[i] << "] average: ";
            cout << ave_vals_type[i] << " (" << ele_quantity[i] << ")\n";
        }
        else {
            cout << "no elements of this type\n";
        }
    }
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void Quality::execute_all(vector<Point3D> &p, vector<Element *> &e){
    
    for (unsigned int i=0; i<e.size(); i++) {
        vector<double> js = e[i]->getJS(p);
        vector<double> jens = e[i]->getJENS(p);
        
        double worst_js = 2, worst_jens=2;
        
        for (unsigned int j=0; j<jens.size(); j++) {
            //cout << " " << jens[j] << "(" << js[j] << ")";
            bool update_jens = false, update_js = false;
            if (worst_jens > jens[j]) {
                worst_jens = jens[j];
                update_jens = true;
            }
            if (worst_js > js[j]) {
                worst_js = js[j];
                update_js = true;
            }
            /*if (update_jens!=update_js) {
                cout <<
                cout << "\njs and jens contradiction at Quality::execute_all\n";
            }*/
        }
        //cout << " -> " << worst_jens << "(" << worst_js << ")\n";
        cout << worst_jens << "\n";

    }
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void Quality::execute_allAR(vector<Point3D> &p, vector<Element *> &e){
    
    for (unsigned int i=0; i<e.size(); i++) {
        cout << "ARf " << e[i]->getAspectRatio(p) << " ARe ";
        //cout << e[i]->getEdgeAspectRatio(p) << "\n";
    }
}

