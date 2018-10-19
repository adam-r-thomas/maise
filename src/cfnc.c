#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "cdef.h"
#include "edef.h"
#include "ndef.h"
#include "cell.h"
#include "nutl.h"
#include "cutl.h"
#include "util.h"
#include "spgr.h"
#include "cfnc.h"

//==================================================================
//  find the dimensionality of the structure: bulk (3) or nano (0)
//==================================================================
int FIND_NDIM(Cell *C)
{
  int i,j,q;

  C->ND = 3;
  LIST(C);
  for(i=0;i<C->N;i++)
    for(j=0;j<C->Nn[i];j++)
      for(q=0;q<3;q++)
	if( abs(C->S[i][j][q])>0.01 )
	  return 3; 
  return 0;
}
//==================================================================
void KILL_DBL(Cell *C, double tol)
{
  int i,j,k,q;

  for(i=0; i<C->N; i++)
    for(q=0;q<D3;q++)
    {
      if( C->X[i][q] < 0.0 )
	C->X[i][q] += 1.0;
      
      if(C->X[i][q]>=1.0)
	C->X[i][q] -= 1.0;
    }
  //===== if the diffference is too close to 0.0 or 1.0 =====
  for(i=0;i<C->N;i++)
    for(j=i+1;j<C->N;j++)
      if( fabs(fabs(fabs(C->X[i][0]-C->X[j][0])-0.5)-0.5)<tol && 
	  fabs(fabs(fabs(C->X[i][1]-C->X[j][1])-0.5)-0.5)<tol && 
	  fabs(fabs(fabs(C->X[i][2]-C->X[j][2])-0.5)-0.5)<tol ) 
      {
	for(k=j;k<C->N-1;k++)
	{
	  for(q=0;q<D3;q++)
	    C->X[k][q] = C->X[k+1][q];
          C->ATMN[k] = C->ATMN[k+1];
	}
	C->N--;
	j--;
      }
}
//==================================================================
void APPL_SG(Cell *C, double tol)
{
  int    i,j,k,q;

  for(j=0,C->N=0;j<C->NSG;j++)
    for(i=0;i<C->NS;i++)
    {
      if(C->N>C->A)
      {
	fprintf(stderr,"Error: the number of atoms exceeds the allocated number %d \n",C->A);
	exit(1);
      }
      for(q=0;q<D3;q++)
	for(k=0, C->X[C->N][q] = C->SG[j][D3*D3+q] ;k<D3;k++)
	  C->X[C->N][q] += C->W[i][k]*C->SG[j][k+D3*q];
      C->ATMN[C->N] = C->ATMN[i];
      for(q=0;q<D3;q++)
      {
	if(C->X[C->N][q] <0.0)
	  C->X[C->N][q] += 1.0;

	if(C->X[C->N][q]>=1.0)
	  C->X[C->N][q] -= 1.0;
      }
      //=====  note the rescaling with the lattice constants  =====
      for(k=0;k<C->N;k++)
	if( fabs(fabs(fabs(C->X[C->N][0]-C->X[k][0])-0.5)-0.5)<tol/C->LAT[0] && 
	    fabs(fabs(fabs(C->X[C->N][1]-C->X[k][1])-0.5)-0.5)<tol/C->LAT[1] && 
	    fabs(fabs(fabs(C->X[C->N][2]-C->X[k][2])-0.5)-0.5)<tol/C->LAT[2] )
	{
          C->N--;
	  break;
	}
      C->N++;
    }
  ORDER(C);
  C->XT = 0;
  Real(C);
  LIST(C);
}
//==================================================================
int FIND_WYC(Cell *C, Cell *D, double tol, char *ISO, int J)
{
  int i,q,k,m,SGN;

  char buf[200],s[200],s1[200],s2[100],w[1000][3];
  FILE *out,*in;

  Relative(C);

  sprintf(buf,"%s/findsym > wyckoff",ISO);
  out = popen(buf,"w");

  fprintf(out,"%s\n",C->TAG);
  fprintf(out,"%lf\n%lf\n0.00\n",tol,tol);
  fprintf(out,"1\n");

  for(i=0;i<3;i++)
  {
    for(q=0;q<D3;q++)
      fprintf(out,"% 4.14lf ",C->L[i][q]);
    fprintf(out,"\n");
  }
  fprintf(out,"1\n");
  fprintf(out,"% 4.14lf % 4.14lf % 4.14lf\n",1.0,0.0,0.0);
  fprintf(out,"% 4.14lf % 4.14lf % 4.14lf\n",0.0,1.0,0.0);
  fprintf(out,"% 4.14lf % 4.14lf % 4.14lf\n",1.0,0.0,1.0);
  fprintf(out,"% d\n",C->N);

  for(i=0;i<C->N;i++)
    if(C->ES[C->ATMN[i]][0]>0)
      fprintf(out,"%s ",C->ES[C->ATMN[i]]);
    else
      fprintf(out,"%d ",C->ATMN[i]+1);
  fprintf(out,"\n");

  for(i=0;i<C->N;i++)
  {
    for(q=0;q<3;q++)
      fprintf(out,"% 4.14lf ",C->X[i][q]);
    fprintf(out,"\n");
  }
  pclose(out);

  out = fopen("str.cif","w");
  fclose(out);
  sprintf(buf,"wyckoff");
  in=fopen(buf,"r");

  system("grep _symmetry_Int_Tables_number wyckoff >> str.cif");

  out = fopen("str.cif","a");

  i = 0;
  while(fgets(buf,100,in))
  {
    if(strstr(buf,"_cell_length_"))
        fprintf(out,"%s",buf);
    if(strstr(buf,"_cell_angle_"))
        fprintf(out,"%s",buf);
    if(strstr(buf,"_symmetry_Int_Tables_number"))
    {
      fprintf(out,"%s",buf);
      sscanf(buf,"%s %d",s1,&SGN);
      Real(C);
      if(J==0)
        return SGN;
      D->SGN = SGN;
      READ_SG(D);
      D->NS = 1;
    }
    if(strstr(buf,"Wyckoff position"))
    {
      sscanf(buf,"%s %s %s",s1,s1,w[i]);
      w[i++][1] = 0;
    }  
    if(strstr(buf,"_atom_site_occupancy"))
    {
      fprintf(out,"_chemical_formula_sum\n");
      fprintf(out,"'");
      for(i=0;i<C->ATMN[C->N-1]+1;i++)
      {
        if(C->SPCZ[0]>0)
        {
          atom_symb(C->SPCZ[i],s1);    
          fprintf(out,"%s ",s1);
        }
        else
          fprintf(out,"%c ",65+i);
      }
      fprintf(out,"'\n");
      fprintf(out,"_atom_site_occupancy wyckoff\n");
      i = 0;
      while(fgets(buf,200,in))
	if(strlen(buf)> 5 && !strstr(buf,"end of cif"))
	{
          sscanf(buf,"%s %s %d %s %lf %lf %lf",s1,s2,&m,w[i],&D->W[0][0],&D->W[0][1],&D->W[0][2]);
	  for(k=0; k<10 && s2[0]!=65+k ;k++);
	  if(C->SPCZ[0]>0)
	    atom_symb(C->SPCZ[k],s);
	  else
	    strcpy(s,s2);
          for(q=0;q<3;q++)
            if(D->W[0][q]<0.0)
              D->W[0][q] += 1.0;
          for(q=0;q<3;q++)
            for(k=0;k<6;k++)
              if(k!=0&&k!=3)
                if(fabs(D->W[0][q]-(double)k/6.0)<0.0002)
                  D->W[0][q] = (double)k/6.0;
          fprintf(out,"%s%s %s  %3d %s % 8.16lf % 8.16lf % 8.16lf\n",s,s1+1,s,m,w[i],D->W[0][0],D->W[0][1],D->W[0][2]);
	}
    }
  }

  fclose(in);
  
  fprintf(out,"loop_\n");
  fclose(out);

  for(i=0;i<C->ATMN[C->N-1]+1;i++)
  {
    if(C->SPCZ[i]>0) 
      atom_symb(C->SPCZ[i],s1); 
    else 
      strcpy(s1,"A");
    sprintf(C->TAG+2*i,"%s ",s1);
  }
  return SGN;
}
//==================================================================
void READ_CIF(Cell *C, char file[], double tol, int NM)    
{ 
  int i,j,n,I,q,k; 

  FILE *in; 
  char buf[200],s[200],t[200],r[200],s0[200],s1[200],s2[200]; 

  if(!(in = fopen(file,"r")))
  {
    fprintf(stderr,"Please provide str.cif file\n");
    exit(1);
  }
  //===== for -cif function get the number of atoms to build Cell =====
  if( C->N==0 )
  {
    while(fgets(buf,200,in)!=0)
    {
      s[0] = 0;
      sscanf(buf,"%s %s",t,s);
      if(strcmp(t,"_atom_site_occupancy")==0)
      {
	fgets(buf,200,in);
	if(strncmp(buf,"_",1)==0)
	  fgets(buf,200,in);
	
	for(C->NS=0;strncmp(buf,"loop_",5)!=0&&strncmp(buf,"#End",4)!=0;C->NS++)
	{
	  sscanf(buf,"%s %s %d %s %s %s %s %s %s",s,r,&i,r,s0,s1,s2,r,r);
	  C->N += i;
	  fgets(buf,200,in);
	}
      }
    }
    fclose(in);
    if( C->N==0 )
    {
      fprintf(stderr,"str.cif does not have proper Wyckoff positions\n");
      exit(0);    
    }
    else
      INIT_CELL(C,"POSCAR",4,NM,0);
  }
  //====================================================================
   
  in = fopen(file,"r");
  n = 0; 
  I = 1;
  while(fgets(buf,200,in)!=0)  
  {
    s[0] = 0;
    sscanf(buf,"%s %s",t,s);

    if(strcmp(t,"_chemical_formula_sum")==0)    // identify element types and quantities
    {
      if(s[0]==0)
	fgets(s,200,in);
      else
	strncpy(s,buf+strlen(t)+1,strlen(buf)-strlen(t));
      s[strlen(s)-2]=0;            // get rid of ' at the end

      for(i=0,C->NTE=0;i<strlen(s);i++) 
        if(s[i] >=65 && s[i] <=90) 
          C->NTE++; 
      C->NSPC = C->NTE;

      for(i=0,j=0;i<strlen(s);j++)  
      {
	C->ES[j][2] = 0;
	sscanf(s+i+1,"%s",t);
	i += strlen(t)+1;
	C->ES[j][0]=t[0];
	C->SL[j] = 2;
	if(t[1] >=97 && t[1] <=122) 
	  C->ES[j][1] = t[1]; 
	else
	{
	  C->ES[j][1] = 0;
	  C->SL[j]--;
	}
	sscanf(t+C->SL[j],"%d",&n);
      }      
      if( strcmp(C->ES[0],"A")==0 )
	C->SPCZ[0] = 0;
      else
	for(i=0;i<C->NSPC;i++)
	  C->SPCZ[i] = symb_atom(C->ES[i]);
    }

    if(strcmp(t,"_symmetry_Int_Tables_number")==0)      // read space group
    { 
      sscanf(s,"%d",&C->SGN); 
      READ_SG(C);
    }

    if(strcmp(t,"_cell_length_a")==0)                    // get unit cell parameters
    {
      sscanf(s,"%lf",&C->LAT[0]);
      C->R0 = 1.0;
    }

    if(strcmp(t,"_cell_length_b")==0) 
      sscanf(s,"%lf",&C->LAT[1]); 
    if(strcmp(t,"_cell_length_c")==0) 
      sscanf(s,"%lf",&C->LAT[2]); 

    if(strcmp(t,"_cell_angle_alpha")==0) 
      sscanf(s,"%lf",&C->ANG[0]); 
    if(strcmp(t,"_cell_angle_beta")==0) 
      sscanf(s,"%lf",&C->ANG[1]); 
    if(strcmp(t,"_cell_angle_gamma")==0) 
    {
      sscanf(s,"%lf",&C->ANG[2]); 
      for(q=0;q<3;q++)
	C->ANG[q] *= Pi/180.0;
      ABC_LT(C);
    }

    if(strcmp(t,"_atom_site_occupancy")==0)              // get Wickoff positions
    {

      fgets(buf,200,in);  
      if(strncmp(buf,"_",1)==0)
	fgets(buf,200,in);

      for(C->NS=0;strncmp(buf,"loop_",5)!=0&&strncmp(buf,"#End",4)!=0;C->NS++)
      {      
	sscanf(buf,"%s %s %s %s %s %s %s %s %s",s,r,r,r,s0,s1,s2,r,r); 
	sscanf(s0,"%lf",&C->W[C->NS][0]);
        sscanf(s1,"%lf",&C->W[C->NS][1]); 
        sscanf(s2,"%lf",&C->W[C->NS][2]); 

        for(q=0;q<3;q++)
          if(C->W[C->NS][q]<0.0)
            C->W[C->NS][q] += 1.0;
        for(q=0;q<3;q++)
          for(k=0;k<6;k++)
            if(k!=0&&k!=3)
              if(fabs(C->W[C->NS][q]-(double)k/6.0)<0.0002)
                C->W[C->NS][q] = (double)k/6.0;

        fgets(buf,200,in); 

	for(i=0;i<C->NTE;i++)
	  if(strncmp(s,C->ES[i],C->SL[i])==0)
	    C->ATMN[C->NS] = i;
	
      }
    }
  }
  fclose(in);
  APPL_SG(C,tol);
  ORDER(C);
  SAVE_CELL(C,"CONV",0);
}  
//==================================================================
//    find multiplicity of sites for the folded u.c.
//==================================================================
int FIND_MTY(Cell *C, double tol)
{
  int i,j,M;

  JAR(C);
  LIST(C);
  for(i=0,M=1;i<C->N;i++)
  {
    for(j=0;j<C->Nn[i];j++)
    {
      if(NDX(C,i,j)>tol)
	break;
      else
	if(C->ATMN[i]!=C->ATMN[C->Ni[i][j]])
	  return 1;
    }
    if(i==0)
      M = j+1;
    if(j+1!=M)
      return 1;
  }

  Relative(C);
  KILL_DBL(C,tol);
  Real(C);

  return M;
}
//==================================================================
//    my algorithm to find the Pearson symbol and PRIM given CONV
//==================================================================
void FIND_PRS(Cell *C, Cell *D, double tol)
{
  int q,k,M;

  abc(C);
  LIST(C);
  RDF(C);
  LIST(D);
  RDF(D);

  if(1.0-CxC(C,D)>0.001)
  {
    Copy_C(D,C);
    sprintf(C->PRS,"xX");
    ORDER(C);
    SAVE_CELL(C,"CONV",0);
    SAVE_CELL(C,"PRIM",0);
    return;
  }
  Copy_C(C,D);

  if(   0 <  C->SGN && C->SGN <   3 ) sprintf(C->PRS,"a");
  if(   2 <  C->SGN && C->SGN <  16 ) sprintf(C->PRS,"m");
  if(  15 <  C->SGN && C->SGN <  75 ) sprintf(C->PRS,"o");
  if(  74 <  C->SGN && C->SGN < 143 ) sprintf(C->PRS,"t");
  if( 142 <  C->SGN && C->SGN < 168 ) sprintf(C->PRS,"h");
  if( 167 <  C->SGN && C->SGN < 195 ) sprintf(C->PRS,"h");
  if( 194 <  C->SGN && C->SGN < 231 ) sprintf(C->PRS,"c");

  sprintf(C->PRS+1,"P");  
  M = 1;

  if( (  2 < C->SGN && C->SGN <  16)|| ( 15 < C->SGN && C->SGN <  75) )  // mS or oS
  {
    for(k=0;k<3;k++)
    {
      Copy_C(C,D);
      for(q=0;q<3;q++)
      {
	D->L[0][q] = 0.5*( C->L[(k+0)%3][q]+C->L[(k+1)%3][q]);
        D->L[1][q] = 0.5*(-C->L[(k+0)%3][q]+C->L[(k+1)%3][q]);
        D->L[2][q] = 1.0*( C->L[(k+2)%3][q]);
	if(k>0)
	  dSwap(&D->L[0][q],&D->L[1][q]);
      }
      if((M=FIND_MTY(D,tol))>1)
	break;
    }
    if(M>1)
      sprintf(C->PRS+1,"S");
  }  

  if(M==1)
  if( ( 15 < C->SGN && C->SGN <  75)|| (194 < C->SGN && C->SGN < 231) )  // oF or cF
  {
    Copy_C(C,D);
    for(q=0;q<3;q++)
    {
      D->L[0][q] = 0.5*(C->L[1][q]+C->L[2][q]);
      D->L[1][q] = 0.5*(C->L[2][q]+C->L[0][q]);
      D->L[2][q] = 0.5*(C->L[0][q]+C->L[1][q]);
    }
    M = FIND_MTY(D,tol);
    if(M>1)
      sprintf(C->PRS+1,"F");
  }

  if(M==1)
  if( ( 15 < C->SGN && C->SGN < 143)|| (194 < C->SGN && C->SGN < 231) )  // oI, tI, or cI
  {
    Copy_C(C,D);
    for(q=0;q<3;q++)
    {
      D->L[0][q] = 0.5*(-C->L[0][q]+C->L[1][q]+C->L[2][q]);
      D->L[1][q] = 0.5*( C->L[0][q]-C->L[1][q]+C->L[2][q]);
      D->L[2][q] = 0.5*( C->L[0][q]+C->L[1][q]-C->L[2][q]);
    }
    M = FIND_MTY(D,tol);
    if(M>1)
      sprintf(C->PRS+1,"I");
  }  

  if(M==1)
  if( (142 < C->SGN && C->SGN < 168) )  // hR
  {
    Copy_C(C,D);
    D->L[0][0] =  0.5*C->LAT[0];  D->L[0][1] = 0.5/sqrt(3.0)*C->LAT[0];  D->L[0][2] = C->LAT[2]/3.0;
    D->L[1][0] = -0.5*C->LAT[0];  D->L[1][1] = 0.5/sqrt(3.0)*C->LAT[0];  D->L[1][2] = C->LAT[2]/3.0;
    D->L[2][0] =  0.0*C->LAT[0];  D->L[2][1] =-1.0/sqrt(3.0)*C->LAT[0];  D->L[2][2] = C->LAT[2]/3.0;
    M = FIND_MTY(D,tol);
    if(M>1)
      sprintf(C->PRS+1,"R");
  }

  if(M==1)
    Copy_C(C,D);

  ORDER(D);
  SAVE_CELL(D,"PRIM",0);
}
//==================================================================
//    compute dot product CxC for two structures based on RDF
//==================================================================
void FIND_CXC(Cell *C, Cell *D, int argc, char **argv)
{
  if(argc > 2)
    C->NM   = D->NM   = ( int  )atoi(argv[2]);
  if(argc > 3)
    C->Rmin = D->Rmin = (double)atof(argv[3]);
  if(argc > 4)
    C->Rmax = D->Rmax = (double)atof(argv[4]);
  if(argc > 5)
    C->DR   = D->DR   = (double)atof(argv[5]);

  LIST(C);
  LIST(D);
  RDF(C);
  RDF(D);
  Print_RDF_FULL(C,"RDF0.dat");
  Print_RDF_FULL(D,"RDF1.dat");

  printf("%lf\n",CxC(C,D));

}
//==================================================================
//    compare RDF, SG, and Volumes for two structures
//==================================================================
void COMP_STR(Cell *C, Cell *D, int argc, char **argv)
{
  int    k;
  double t,o;

  if(argc > 2)
    C->NM   = D->NM   = ( int  )atoi(argv[2]);
  if(argc > 3)
    C->Rmin = D->Rmin = (double)atof(argv[3]);
  if(argc > 4)
    C->Rmax = D->Rmax = (double)atof(argv[4]);
  if(argc > 5)
    C->DR   = D->DR   = (double)atof(argv[5]);

  C->ND = FIND_NDIM(C);
  D->ND = FIND_NDIM(D);

  LIST(C);
  Print_List(C);
  system("mv list.dat list0.dat");
  LIST(D);
  Print_List(D);
  system("mv list.dat list1.dat");

  RDF(C);
  RDF(D);
  Print_RDF_FULL(C,"RDF0.dat");
  Print_RDF_FULL(D,"RDF1.dat");
  
  printf("\n");
  printf("  STR     vol/atom           space group number           RDF scalar product\n");
  printf(" number   A^3/atom     10^-1   10^-2   10^-4   10^-8         0          1  \n\n");

  t = o = 0.1;
  printf("   %d  % 12.6lf   ",0,Cell_VOLUME(C)/(double)C->N);
  for(k=1;k<8;k+=2)
  {
    printf("  % 4d  ",FIND_WYC(C,D,t,ISOPATH,0));
    t*= o;
    o*= o;
  }
  printf("  % 10.6lf % 10.6lf\n",CxC(C,C),CxC(C,D));
  t = o = 0.1;
  printf("   %d  % 12.6lf   ",1,Cell_VOLUME(D)/(double)D->N);
  for(k=1;k<8;k+=2)
  {
    printf("  % 4d  ",FIND_WYC(D,D,t,ISOPATH,0));
    t*= o;
    o*= o;
  }
  printf("  % 10.6lf % 10.6lf\n",CxC(D,C),CxC(D,D));

  printf("\n");

}
//==================================================================
//   builds and initializes Cell for FLAG operations
//==================================================================
void INIT_CELL(Cell *C, char filename[], int N, int NM, int J)
{
  FILE *in;
  C->MODT = 0;
  C->A    = 0;

  if(J==1)
  {
    if((in=fopen(filename,"r"))==0)
    {
      perror(filename);
      exit(1);
    }

    READ_CELL(C,filename);
  }
    
  C->N   *= N;
  C->A    = C->N;
  C->NM   = NM;
  C->ND   = 3;
  C->Rc   = 4.2;
  C->Rmax = C->Rc;
  C->Rmin = 0.99*C->Rmax;
  C->DR   = 0.008;
  C->NP   = 1;
  C->NB   = 1;

  Build_Cell(C,0);
  if(J==1)
    READ_CELL(C,filename);
}
//==================================================================
//    run a job defined by FLAGs
//==================================================================
void CELL_EXAM(Cell *C, Cell *D, int argc, char **argv)
{
  double tol,L;
  int    i,NM,N[3];

  NM = 300;

  //================   list available options  ===============
  if(argc<2)
  {
    printf("For specified JOBT = 0 you should provide a FLAG:\n\n");
    printf("-cxc    compute dot product for POSCAR0 and POSCAR1 using RDF           \n");
    printf("-cmp    compare RDF, space group, and volume of POSCAR0 and POSCAR1     \n");
    printf("-wyc    convert POSCAR into str.cif, CONV, PRIM                         \n");
    printf("-cif    convert str.cif into CONV and PRIM                              \n");
    printf("-rot    rotate  a nanoparticle along eigenvectors of moments of inertia \n");
    printf("-dim    find    whether POSCAR is periodic (3) or non-periodic (0)      \n");
    printf("-box    reset   the box size for nanoparticles                          \n");
    printf("-sup    make    a supercell specified by na x nb x nc                   \n");
    printf("-vol    compute volume per atom for crystal or nano structures          \n");
    exit(0);
  }

  //=============  determine Wyckoff positions  =============
  if(strncmp(argv[1],"-wyc",4)==0)
  {
    tol = 0.01;
    if(argc>2)
      tol = (double)atof(argv[2]);
    INIT_CELL(C,"POSCAR",4,NM,1);
    INIT_CELL(D,"POSCAR",4,NM,1);
    FIND_WYC(C,D,tol,ISOPATH,1);
    READ_CIF(C,"str.cif",tol,NM);
    FIND_PRS(C,D,tol);
    printf("%3d %s%d\n",C->SGN,C->PRS,C->N);
    exit(0);
  }
   //================  convert cif into CONV  ================
  if(strncmp(argv[1],"-cif",4)==0)
  {
    tol = 0.01;
    if(argc>2)
      tol = (double)atof(argv[2]);
    C->N = 0;
    READ_CIF(C,"str.cif",tol,NM);    
    exit(0);
  }
  //================  compare RDF of 2 structures  ==========
  if(strncmp(argv[1],"-cxc",4)==0)
  {
    if(argc>2)
      NM = (int)atoi(argv[2]);
    INIT_CELL(C,"POSCAR0",1,NM,1);
    INIT_CELL(D,"POSCAR1",1,NM,1);
    FIND_CXC(C,D,argc,argv);
    exit(0);
  }
  //================  compare RDF of 2 structures  ==========
  if(strncmp(argv[1],"-cmp",4)==0)
  {
    if(argc>2)
      NM = (int)atoi(argv[2]);
    INIT_CELL(C,"POSCAR0",1,NM,1);
    INIT_CELL(D,"POSCAR1",1,NM,1);
    COMP_STR(C,D,argc,argv);
    exit(0);
  }
  //================  rotate a nanoparticle  ================
  if(strncmp(argv[1],"-rot",4)==0)
  {
    INIT_CELL(C,"POSCAR",1,NM,1);
    C->ND = FIND_NDIM(C);
    CENTER(C,0.0);
    if( C->ND!= 0)
    {
      fprintf(stderr,"POSCAR is periodic\n");
      exit(1);
    }
    NANO_ROT(C,1);
    exit(0);
  }
  //================  determine dimensionality  =============
  if(strncmp(argv[1],"-dim",4)==0)
  {
    if(argc>2)
      C->Rc = (double)atof(argv[2]);
    INIT_CELL(C,"POSCAR",1,NM,1);
    printf("% 3d\n",FIND_NDIM(C));
    exit(0);
  }
  //================  resize the nanparticle box  ===========
  if(strncmp(argv[1],"-box",4)==0)
  {
    INIT_CELL(C,"POSCAR",1,NM,1);
    C->ND = FIND_NDIM(C);
    if( C->ND!= 0)
      printf("WARNING POSCAR is periodic\n");

    if( argc<3 )
    {
      fprintf(stderr,"Please provide new box size\n");
      exit(0);
    }
    L = (double)atof(argv[2]);
    for(i=0;i<3;i++)
      C->L[i][i] = L;
    SAVE_CELL(C,"CONTCAR",0);
    exit(0);
  }
  //===================== make a supercell  ==================
  if(strncmp(argv[1],"-sup",4)==0)
  {
    N[0] = N[1] = N[2] = 1;
    if(argc>2)
      N[0] = (int)atoi(argv[2]);
    if(argc>3)
      N[1] = (int)atoi(argv[3]);
    if(argc>4)
      N[2] = (int)atoi(argv[4]);
    INIT_CELL(C,"POSCAR",N[0]*N[1]*N[2],NM,1);
    INIT_CELL(D,"POSCAR",1,NM,1);
    Clone(C,D,N[0],N[1],N[2]);
    SAVE_CELL(C,"CONTCAR",0);
    exit(0);
  }
  //================  compute volume per atom  ================
  if(strncmp(argv[1],"-vol",4)==0)
  {
    INIT_CELL(C,"POSCAR",1,NM,1);
    C->ND = FIND_NDIM(C);
    printf("% lf\n",Cell_VOLUME(C)/(double)C->N);
    exit(0);
  }

  //================   list available options  ===============

  printf("\nThe FLAG is not recognized. Allowed FLAGS are:\n\n");
  printf("-cxc    compute dot product for POSCAR0 and POSCAR1 using RDF           \n");
  printf("-cmp    compare RDF, space group, and volume of POSCAR0 and POSCAR1     \n");
  printf("-wyc    convert POSCAR into str.cif, CONV, PRIM                         \n");
  printf("-cif    convert str.cif into CONV and PRIM                              \n");
  printf("-rot    rotate  a nanoparticle along eigenvectors of moments of inertia \n");
  printf("-dim    find    whether POSCAR is periodic (3) or non-periodic (0)      \n");
  printf("-box    reset   the box size for nanoparticles                          \n");
  printf("-sup    make    a supercell specified by na x nb x nc                   \n");
  printf("-vol    compute volume per atom for crystal or nano structures          \n");
 
 exit(0);
}
//==================================================================
