#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

int main(int argc, char* argv[]) {

   string sdcd, infile, sbegframe, sendframe;
   if (argc == 5) { sdcd = argv[1]; infile = argv[2]; sbegframe = argv[3]; sendframe = argv[4];  }
      else { cout << ">> Please provide dcd trajectory, rdf input file, and first and last frame to be analyzed\n>> Usage: ./exec traj.dcd rdf.in first_frame last_frame\n"; return 0; }

///// Read an input file with atom numbers of ref pts and prx atoms, etc. /////   
   ifstream inf;
   inf.open(infile.c_str(), ios::in);
   string infline;
   int i=0; 
   double ref[10];// Number of ref pts is 10 for now

  while (inf.is_open()) {
     getline (inf, infline);
     if (inf.eof()) { break; }
     i++;
   }
   int nline = i;
   inf.clear(); inf.seekg(0, ios::beg);

   int k = 0;
   string lines[nline]; 

   while (inf.is_open() && k < nline) {
    getline (inf, infline);
    lines[k] = infline;  
    k++;
   }

   //int refbeg = atoi(lines[6].c_str());
   //int refend = atoi(lines[8].c_str());
   //int refnatom = (refend-refbeg)+1;
   //cout << "refbeg= " << refbeg<<" refend= "<< refend<<"refnatom= "<<refnatom<<endl;
   int nmol = atoi(lines[1].c_str());
   int freq = atoi(lines[3].c_str());   
   cout << ">> " << nmol << " molecules in rdf\n>> Reading every " << freq << " frames\n";

   double mol[nmol];
   for (int j=0; j<nmol; j++) { 
     mol[j] = atof(lines[j+18].c_str());  // Reads in RDF moleclue atom nums
     cout << "mol " <<j+1<< " atom is " << mol[j] << endl; 
    }

// TO DO: Make its measure to all DNA atoms, not just 10 reference points
   for (int r=0; r<10; r++) {
      ref[r] = atof(lines[r+6].c_str());
      cout << "ref atom" <<r+1<< "is " << ref[r] << endl;
    }
   inf.close();

///// Reading DCD trajectory ////// 
   ifstream dcd;
   dcd.open(sdcd.c_str(), ios::in|ios::binary|ios::ate);
   long length; 
   length = dcd.tellg();

   cout << ">> Size of file is " << length/(1024*1024) << " MB.\n";
   char * buffer = new char[length];
   dcd.seekg(0, ios::beg);
   cout << "** Reading DCD file\n";
   long headlen = 0;
   int tmp, natoms;
   dcd.read((char*)&tmp, sizeof(int));
   headlen = headlen + 8 + tmp;
   dcd.seekg(headlen);
   dcd.read((char*)&tmp, sizeof(int));
   headlen = headlen + 8 + tmp;
   dcd.seekg(headlen);
   dcd.read((char*)&tmp, sizeof(int));
   headlen = headlen + 8 + tmp;
   dcd.read((char*)&natoms, sizeof(int));
   long framelen = natoms*12 + 80;
   long nframe = ((length-headlen)/framelen);

   cout << ">> " << natoms << " atoms in dcd file\n>> " << nframe << " frames in dcd file\n";
  
   double cx, cy, cz; // Unit cell dimensions
   float coords[natoms][3][nframe];
   for (int i=0;i<nframe;i++) {
     dcd.seekg(long(headlen) + framelen*i + 4); 
     dcd.read((char*)&cx, sizeof(double));
     dcd.seekg(long(headlen) + framelen*i + 20);
     dcd.read((char*)&cy, sizeof(double));
     dcd.seekg(long(headlen) + framelen*i + 44);
     dcd.read((char*)&cz, sizeof(double));

     dcd.seekg(long(headlen) + framelen*i + 56);
     dcd.read((char*)coords, sizeof(float)); // What is coords here, could it just as well be some other float var? Try

     for (int j=0;j<3;j++) {
       for (int k=0;k<natoms;k++) {
         dcd.read((char*)&coords[k][j][i], sizeof(float));
         }
      dcd.read((char*)&tmp, sizeof(float));
      dcd.read((char*)&tmp, sizeof(float));
      }
     }

   dcd.close();
   delete[] buffer;
   int bins[nmol][11]; // Create our bins
   for (int z=0; z<nmol; z++) { 
      for (int y=0; y<11; y++) { bins[z][y] = 0; } // Set all bin values to 0 first
    }
   int nmeasured = 0;
   int begframe = atoi(sbegframe.c_str());
   int endframe = atoi(sendframe.c_str());
   if (endframe > nframe) { endframe = nframe; }   

   for (int j=begframe; j<endframe; j+=freq) {
     double totdist = 0;
     cout << "---------------------------\n** Reading frame " << j+1 << endl << "---------------------------\n";
     for (int k = 0; k<nmol; k++) { 
       double mindist = 1000; 
       int currentmol = mol[k]-1;   
       nmeasured++;
       for (int l=0; l<10; l++) {
         int refpt = ref[l]-1;
         double dx2 = pow((coords[refpt][0][j]-coords[currentmol][0][j]), 2);
         double dy2 = pow((coords[refpt][1][j]-coords[currentmol][1][j]), 2);
         double dz2 = pow((coords[refpt][2][j]-coords[currentmol][2][j]), 2);
         double dist = sqrt(dx2 + dy2 + dz2);
         if (dist < mindist) { mindist = dist; }        
       }
     if      (mindist <= 10)                   { bins[k][0] ++; }
     else if (mindist > 10  && mindist <= 20)  { bins[k][1] ++; }
     else if (mindist > 20 && mindist <= 30)  { bins[k][2] ++; }
     else if (mindist > 30 && mindist <= 40)  { bins[k][3] ++; }
     else if (mindist > 40 && mindist <= 50)  { bins[k][4] ++; }
     else if (mindist > 50 && mindist <= 60)  { bins[k][5] ++; }
     else if (mindist > 60 && mindist <= 70)  { bins[k][6] ++; }
     else if (mindist > 70 && mindist <= 80)  { bins[k][7] ++; }
     else if (mindist > 80 && mindist <= 90)  { bins[k][8] ++; }
     else if (mindist > 90 && mindist <=100)  { bins[k][9] ++; }
     else if (mindist > 100 && mindist <=110)  { bins[k][10] ++;}
     totdist += mindist;
     //cout << "MOL " << k+1 << " is " << mindist << "A away from DNA in frame " << j+1 << endl;
     }
     double avgdist = totdist/nmol;
     cout << ">> Average MOL distance from DNA in frame " <<j+1<< " is " << avgdist << " A\n";
    }

   for (int i=0; i<nmol; i++) { cout << "MOL " << i+1 << " rdf: " << endl;
       for (int j=0; j<11; j++) { cout << j*10 << " to " << (j+1)*10 << ": " << bins[i][j] << endl; }
   }
   // Calculate 22 bins that hold sum of all MOLs in each bin
   int totbin[11];
   for (int t=0; t<11; t++) { totbin[t] = 0; } // Set all bins to 0 first
   for (int i=0; i<11; i++) { 
     for (int j=0; j<nmol; j++) { totbin[i] += bins[j][i]; }
    cout << ">> Bin " << i+1 << " total: " << totbin[i] << endl;
   }
   cout << "Total MOL distances measured: " << nmeasured << endl;
   
return 0;
}