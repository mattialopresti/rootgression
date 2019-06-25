#include "TCanvas.h"
#include "TEllipse.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH2F.h"
#include "TH1F.h"
#include "TLine.h"
#include "TH3F.h"
#include "TLegend.h"
#include "TMath.h"
#include "Math/SpecFuncMathCore.h"
#include "TMatrix.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TText.h"
#include "TVector.h"
#include "TVectorD.h"
#include "TVectorT.h"
#include "TVirtualFitter.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX 256
Float_t t;		//T of student for 3 replicas 95% of confidence
Int_t lineBuffer=999;		//Line buffer
Int_t error=0;			//Number of errors overall

void which_t(Int_t rep);

void rootgression(const Char_t *inputfile){

	/*======= Variables declaration =======*/
	printf("\nOpening input file '%s'.\n",inputfile); 		//Check file
	FILE *ifp;							//Input file name
	Char_t const *mode = "r"; 					// Read
	Float_t i=0, j=0, k=0, x=0;					//Runners reset on every cycle
	Int_t z=0;
	Int_t p=0, rep;						//Parameters of the model
	Float_t MIN_f1, MAX_f1, MIN_f2, MAX_f2, MIN_f3, MAX_f3, MIN_f4, MAX_f4;
	Double_t f1[MAX], f2[MAX];	
	Double_t mean, variance=0, spe, sb, sb_crit;			//Errore puramente sperimentale corretto per t value
	Int_t factors, experiments, matrix_columns; 			//Number of studied factors, number of experiments done and total number of columns (2^factors)
   	Char_t *line = NULL;						//Blank line string
    	ssize_t read;							//Runner over lines of the input
    	size_t len = 0; 						//Length of each line
	
	/*======= Number of factors and experiments =======*/
	do{								//Scan for Factor number for matrix initialization
		printf("Insert a valid factor number:	");
		scanf("%d",&factors);
	}while(factors<1 || factors>4);
	do{								//Scan for experiments number as can be greater or lower than columns
		printf("Insert a valid experiments number:	");
		scanf("%d",&experiments);
	}while(experiments<1);
	do{								//Scan for experiment replicas
		printf("Insert the number of replicas that were done:	");
		scanf("%d",&rep);
	}while(experiments<1);
	matrix_columns=pow(2,factors);					//Columns are 2^factors
	TArrayD rep_array(rep);
	for(i=0;i<rep;i++){
		printf("Please insert the %f replica:	",i+1);
		scanf("%lf",&rep_array[i]);
	}
	which_t(rep);

	/*======= Matrix, vectors and arrays =======*/
	TMatrixD doe(experiments,matrix_columns); 			//Experimental matrix, first column 1 for intercept
	TMatrixD doe_trans(matrix_columns,experiments); 		//Transposed matrix, first row 1 for intercept
	TMatrixD information(matrix_columns,matrix_columns); 		//Information columns
	TMatrixD doe_disp(matrix_columns,matrix_columns); 		//Covariance matrix
	TMatrixD model(matrix_columns,1);				//Model column vector matrix
	TMatrixD answers(experiments,1);				//Answers column vector matrix
	TMatrixD predicted(experiments,1);				//Predicted column vector matrix
	TMatrixD residuals(experiments,1);				//Residual columns vector matrix
	TMatrixD residuals_trans(1,experiments);			//Residual row vector matrix
	TMatrixD matr_tmp(matrix_columns, experiments); 		//Temporary matrix columns
	TMatrixD vettore_tmp(1,experiments);				//Runner column vector
	TArrayD arr_tmp(experiments);					//Runner array
	TArrayD arr_tmp_1(experiments);					//Runner array
	TArrayD arr_tmp_2(experiments);					//Runner array
	TArrayD arr_f1(experiments);					//Factor 1 array
	TArrayD arr_f2(experiments);					//Factor 2 array
	TArrayD arr_f3(experiments);					//Factor 3 array
	TArrayD arr_f4(experiments);					//Factor 4 array
	TArrayD arr_f1_rs(experiments);					//Factor 1 array range scaled
	TArrayD arr_f2_rs(experiments);					//Factor 2 array range scaled
	TArrayD arr_f3_rs(experiments);					//Factor 3 array range scaled
	TArrayD arr_f4_rs(experiments);					//Factor 4 array range scaled
	TArrayD arr_answers(experiments);				//Answers array	

	/*======= Opening the file =======*/
	printf("\nOpening input file '%s'.\n",inputfile);		 //Check file
	ifp = fopen(inputfile, mode);					//Opening the file
	if (ifp == NULL) {						//Warning can't open file
		fprintf(stderr, "\n  ERROR: Can't open input file!\n");
  		error=1;
	}
	else{
        	i++;
		printf("\nInput file opened successfully.\n");		 //File opened
	}

	/*======= Reading line by line =======*/
	i=0;
	if(factors==1){
		printf("Insert first factor's minimum:	");
		scanf("%f",&MIN_f1);	
		printf("Insert first factor's maximum:	");
		scanf("%f",&MAX_f1);
	    	while ((read = getline(&line, &len, ifp)) != -1) {
			if (strlen(line) > 1) { 			//Skip null lines
				sscanf(line, "%lf %lf",&arr_f1[i],&arr_answers[i]); 			//Reading 1 factor + answer
				arr_f1_rs[i] = ((((arr_f1[i]- MIN_f1) / (MAX_f1-MIN_f1)) *2 ) -1); 	// Factor 1 Range Scaled
				i++;
	       		}
	    	}
	}
	if(factors==2){
		printf("Insert first factor's minimum:	");
		scanf("%f",&MIN_f1);	
		printf("Insert first factor's maximum:	");
		scanf("%f",&MAX_f1);
		printf("Insert second factor's minimum:	");
		scanf("%f",&MIN_f2);	
		printf("Insert second factor's maximum:	");
		scanf("%f",&MAX_f2);
	    	while ((read = getline(&line, &len, ifp)) != -1) {
			if (strlen(line) > 1) { 			//Skip null lines
				sscanf(line, "%lf %lf %lf",&arr_f1[i],&arr_f2[i],&arr_answers[i]); 	//Reading 2 factors + answer
				arr_f1_rs[i] = ((((arr_f1[i]- MIN_f1) / (MAX_f1-MIN_f1)) *2 ) -1); 	//Factor 1 Range Scaled
				arr_f2_rs[i] = ((((arr_f2[i]- MIN_f2) / (MAX_f2-MIN_f2)) *2 ) -1); 	//Factor 2 Range Scaled
				i++;
	       		}
	    	}
	}
	if(factors==3){
		printf("Insert first factor's minimum:	");
		scanf("%f",&MIN_f1);	
		printf("Insert first factor's maximum:	");
		scanf("%f",&MAX_f1);
		printf("Insert second factor's minimum:	");
		scanf("%f",&MIN_f2);	
		printf("Insert second factor's maximum:	");
		scanf("%f",&MAX_f2);
		printf("Insert third factor's minimum:	");
		scanf("%f",&MIN_f3);	
		printf("Insert third factor's maximum:	");
		scanf("%f",&MAX_f3);
	    	while ((read = getline(&line, &len, ifp)) != -1) {
			if (strlen(line) > 1) { 			//Skip null lines
				sscanf(line, "%lf %lf %lf %lf",&arr_f1[i],&arr_f2[i],&arr_f3[i],&arr_answers[i]); //Reading 3 factors + answer
				arr_f1_rs[i] = (((arr_f1[i]- MIN_f1) / (MAX_f1-MIN_f1)) *2 ) -1;
				arr_f2_rs[i] = (((arr_f2[i]- MIN_f2) / (MAX_f2-MIN_f2)) *2 ) -1;
				arr_f3_rs[i] = (((arr_f3[i]- MIN_f3) / (MAX_f3-MIN_f3)) *2 ) -1;
				i++;
	       		}
	    	}
	}
	if(factors==4){
		printf("Insert first factor's minimum:	");
		scanf("%f",&MIN_f1);	
		printf("Insert first factor's maximum:	");
		scanf("%f",&MAX_f1);
		printf("Insert second factor's minimum:	");
		scanf("%f",&MIN_f2);	
		printf("Insert second factor's maximum:	");
		scanf("%f",&MAX_f2);
		printf("Insert third factor's minimum:	");
		scanf("%f",&MIN_f3);	
		printf("Insert third factor's maximum:	");
		scanf("%f",&MAX_f3);
		printf("Insert fourth factor's minimum:	");
		scanf("%f",&MIN_f4);	
		printf("Insert fourth factor's maximum:	");
		scanf("%f",&MAX_f4);
	    	while ((read = getline(&line, &len, ifp)) != -1) {
			if (strlen(line) > 1) { 			//Skip null lines
				sscanf(line, "%lf %lf %lf %lf %lf",&arr_f1[i],&arr_f2[i],&arr_f3[i],&arr_f4[i],&arr_answers[i]); //Reading 4 factors + answer
				arr_f1_rs[i] = (((arr_f1[i]- MIN_f1) / (MAX_f1-MIN_f1)) *2 ) -1;
				arr_f2_rs[i] = (((arr_f2[i]- MIN_f2) / (MAX_f2-MIN_f2)) *2 ) -1;
				arr_f3_rs[i] = (((arr_f3[i]- MIN_f3) / (MAX_f3-MIN_f3)) *2 ) -1;
				arr_f4_rs[i] = (((arr_f4[i]- MIN_f4) / (MAX_f4-MIN_f4)) *2 ) -1;
				i++;
	       		}
	    	}
	}
    	fclose(ifp);							//Closing the file

	/*======= Computing DOE matrix =======*/
	//1 Factor
	if(factors>=1){
		//Column 0
		for(i=0; i<experiments; i++){
			arr_tmp[i]=1;
		}
		vettore_tmp.SetMatrixArray(arr_tmp.GetArray());
		TMatrixDColumn(doe,0) = TMatrixDRow(vettore_tmp,0);
		//Column 1 - Factor 1
		vettore_tmp.SetMatrixArray(arr_f1_rs.GetArray());
		TMatrixDColumn(doe,1) = TMatrixDRow(vettore_tmp,0);
	}
	//2 Factors
	if(factors>=2){
		//Column 2 - Factor 2
		vettore_tmp.SetMatrixArray(arr_f2_rs.GetArray());
		TMatrixDColumn(doe,2) = TMatrixDRow(vettore_tmp,0);
		//Column 3 - Factor 1x2
		TMatrixDColumn(doe,3) = TMatrixDColumn(doe,1);
		TMatrixDColumn(doe,3) *= TMatrixDColumn_const(doe,2);
	}
	//3 Factors
	if(factors>=3){
		//Column 4 - Factor 3
		vettore_tmp.SetMatrixArray(arr_f3_rs.GetArray());
		TMatrixDColumn(doe,4) = TMatrixDRow(vettore_tmp,0);
		//Column 5 - Factor 1x3
		TMatrixDColumn(doe,5) = TMatrixDColumn(doe,1);
		TMatrixDColumn(doe,5) *= TMatrixDColumn_const(doe,4);
		//Column 6 - Factor 2x3
		TMatrixDColumn(doe,6) = TMatrixDColumn(doe,2);
		TMatrixDColumn(doe,6) *= TMatrixDColumn_const(doe,4);
		//Column 7 - Factor 1x2x3
		TMatrixDColumn(doe,7) = TMatrixDColumn(doe,1);
		TMatrixDColumn(doe,7) *= TMatrixDColumn_const(doe,2);
		TMatrixDColumn(doe,7) *= TMatrixDColumn_const(doe,4);
	}
	//4 Factors
	if(factors>=4){
		//Colonna 8
		vettore_tmp.SetMatrixArray(arr_f4_rs.GetArray());
		TMatrixDColumn(doe,8) = TMatrixDRow(vettore_tmp,0);
		//Colonna 9
		TMatrixDColumn(doe,9) = TMatrixDColumn(doe,1);
		TMatrixDColumn(doe,9) *= TMatrixDColumn_const(doe,8);
		//Colonna 10
		TMatrixDColumn(doe,10) = TMatrixDColumn(doe,2);
		TMatrixDColumn(doe,10) *= TMatrixDColumn_const(doe,8);
		//Colonna 11
		TMatrixDColumn(doe,11) = TMatrixDColumn(doe,4);
		TMatrixDColumn(doe,11) *= TMatrixDColumn_const(doe,8);
		//Colonna 12
		TMatrixDColumn(doe,12) = TMatrixDColumn(doe,1);
		TMatrixDColumn(doe,12) *= TMatrixDColumn_const(doe,2);
		TMatrixDColumn(doe,12) *= TMatrixDColumn_const(doe,8);
		//Colonna 13
		TMatrixDColumn(doe,13) = TMatrixDColumn(doe,1);
		TMatrixDColumn(doe,13) *= TMatrixDColumn_const(doe,4);
		TMatrixDColumn(doe,13) *= TMatrixDColumn_const(doe,8);
		//Colonna 14
		TMatrixDColumn(doe,14) = TMatrixDColumn(doe,2);
		TMatrixDColumn(doe,14) *= TMatrixDColumn_const(doe,4);
		TMatrixDColumn(doe,14) *= TMatrixDColumn_const(doe,8);	
		//Colonna 15
		TMatrixDColumn(doe,15) = TMatrixDColumn(doe,1);
		TMatrixDColumn(doe,15) *= TMatrixDColumn_const(doe,2);
		TMatrixDColumn(doe,15) *= TMatrixDColumn_const(doe,4);
		TMatrixDColumn(doe,15) *= TMatrixDColumn_const(doe,8);	
	}

	/*======= Computing matrix calculations to find model  =======*/
	printf("Experimental matrix\n"); //Experimental matrix
	doe.Print();
	printf("Transposed experimental matrix\n"); //Transposed experimental matrix
	doe_trans.Transpose(doe);
	doe_trans.Print();
	printf("Information matrix\n"); //Information matrix
	information = doe_trans * doe;		
	information.Print();
	printf("Dispersion matrix\n"); //Dispersion matrix
	doe_disp = information;
	doe_disp.Invert();
	i = doe_disp.Determinant();
	printf("Determinant of dispersion matrix: %lf\n",i);	//Determinant of dispersion matrix
	doe_disp.Print();
	printf("Model parameters");
	vettore_tmp.SetMatrixArray(arr_answers.GetArray());	//Model calculation
	TMatrixDColumn(answers,0) = TMatrixDRow(vettore_tmp,0);
	matr_tmp = doe_disp * doe_trans;
	model = matr_tmp * answers;	
	mean = TMath::Mean(rep, &rep_array[0]);
	for(i=0;i<rep;i++){
		variance = variance + pow(rep_array[i]-mean,2);
	}
	variance = variance /(rep-1);
	spe=sqrt(variance);
	printf("Answers:");
	sb = sqrt(pow(spe,2)/experiments);		//Calculation of sb (halved because model parameters, not effects)
	sb_crit = sb*t;					//Calculation of sb_crit
	if(model(0,0)<(sb_crit*2) && model(0,0)>-(sb_crit*2) ){
			model(0,0)=0;
			p++;
	}
	for(i=1;i<matrix_columns;i++){
		if(model(i,0)<sb_crit && model(i,0)>-sb_crit ){
			model(i,0)=0;
			p++;
		}
	}
	p=i-p;
	model.Print();

	/*======= Model diagnostics =======*/
	TCanvas *c1 = new TCanvas("c1","Dispersion matrix",0,0,600,400); //Dispersion matrix plot
	doe_disp.Draw("surf1");
	c1->Draw();
	predicted = doe * model;
	printf("Answers:");
	answers.Print();	//Answers
	printf("Predicted:");
	predicted.Print();	//Predicted
	residuals = answers - predicted;
	printf("Residuals:");
	residuals.Print();	//Residuals	
	TCanvas *c2 = new TCanvas("c2","Residuals",0,0,600,400); //Residual plot
	auto h1 = new TH1F ("h1","Residual plot",experiments,0,experiments);
	residuals_trans.Transpose(residuals);
	for(i=0; i<experiments; i++){
		h1->Fill(i,residuals_trans(0,i));
	}
	TLine *zero = new TLine(0,0,experiments,0);
	zero->SetLineColor(kRed);
	h1->Draw("*");
	zero->Draw();
	c1->Draw();

	//Statistic parameters
	printf("n of Replicas:			%d\n",rep); //Replicas
	printf("Mean:				%lf\n",mean); //Mean
	printf("Variance:			%lf\n",variance); //Variance
	printf("Experimental error:		%lf\n",spe); //Experimental error
printf("Coefficient of variation:		%lf\n",spe/mean); //CV%
	printf("t value:			%f\n", t); //T-value
	printf("Uncertainty of the esteem:	%lf\n",sb); //Uncertainty of the esteem
	printf("Critical uncertainty:		%lf\n\n",sb_crit); //Crit unc
	
	//Declaration of all the sum of squares and arrays
	Double_t SS_T=0;
	Double_t SS_Mean=0;
	Double_t SS_Corr=0;
	Double_t SS_Fact=0;
	Double_t SS_Res=0;
	//Total sum of squares
	for(i=0; i<experiments;i++){
		SS_T = SS_T+pow(arr_answers[i],2);
	}
	printf("SS_Tot: %lf				Degrees of freedom: %d\n",SS_T,experiments); //Print total sum of squares, freedom degrees
	//Sum of squares due to the mean
	SS_Mean = TMath::Mean(experiments, &arr_answers[0]);
	printf("SS_Mean: %lf				Degrees of freedom: 1\n",SS_Mean); //Print sum of squares due to the mean, freedom degrees
	for(i=0; i<experiments;i++){
		SS_Corr = SS_Corr+ pow(arr_answers[i]-SS_Mean,2);
	}
	printf("SS_Corr: %lf				Degrees of freedom: %d\n",SS_Corr, experiments-1); //Print sum of squares corrected for the mean, freedom degrees
	for(i=0; i<experiments;i++){
		SS_Fact = SS_Fact+ pow(predicted(i,0)-SS_Mean,2);
	}
	printf("SS_Fact: %lf				Degrees of freedom: %d\n",SS_Fact, p-1); //Print factor sum of squares, freedom degrees
	for(i=0; i<experiments;i++){
		SS_Res = SS_Res+ pow(residuals(i,0),2);
	}
	printf("SS_Res: %lf				Degrees of freedom: %d\n",SS_Res, experiments-p); //Print sum of squares corrected for the mean, freedom degrees
	printf("\n\nR^2: 						%lf\n",SS_Fact/SS_Corr); //Correlation coefficient of the model
	i = 1- (SS_Res/(experiments-p))/(SS_Corr/(experiments-1));
	printf("R^2 adj: 					%lf\n",i); //Correlation coefficient adjusted of the model
	i = (SS_Corr/(rep-1))/(SS_Res/(experiments-rep));
	printf("F test on significance via residuals: 		%lf\n",i); //F test on significance 
	i = pow((model(0,0)-mean),2)/(variance*(pow(rep,-1)-pow(experiments,-1)));
	printf("F test on curvature via variance: 		%lf\n",i); //F test on significance 

	/*======= Experimental domain graphs =======*/
	for(i=0; i<experiments; i++){
		arr_tmp[i]= arr_tmp_1[i] = arr_tmp_2[i] = 1;
	}
	if(factors==1){						//1 Factors experimental domain plot
		TCanvas *c3 = new TCanvas("c3","Experimental domain",0,0,600,400);
		TGraph2D *monodimensional = new TGraph2D(experiments*experiments);
		monodimensional->SetTitle("Experimental domain; Factor 1; ; ");
		monodimensional->SetNpy(experiments);
		monodimensional->SetNpx(experiments);
		for (x=0; x<experiments; x++) {
 			monodimensional->SetPoint(x,arr_f1_rs[x],arr_tmp_1[x],arr_tmp_2[x]);
		}
		gStyle->SetPalette(1);
		monodimensional->SetMarkerStyle(20);
		monodimensional->Draw("pcol");
	}
	if(factors==2){						//2 Factors experimental domain plot
		TCanvas *c3 = new TCanvas("c3","Experimental domain",0,0,600,400);
		TGraph2D *bidimensional = new TGraph2D(experiments*experiments);
		bidimensional->SetTitle("Experimental domain; Factor 1; Factor 2; ");
		bidimensional->SetNpy(experiments);
		bidimensional->SetNpx(experiments);
		for (x=0; x<experiments; x++) {
 			bidimensional->SetPoint(x,arr_f1_rs[x],arr_f2_rs[x],arr_tmp_2[x]);
		}
		gStyle->SetPalette(1);
		bidimensional->SetMarkerStyle(20);
		bidimensional->Draw("pcol");
	}
	if(factors==3){ 					//3 Factors experimental domain plot
		TCanvas *c3 = new TCanvas("c3","Experimental domain",0,0,600,400);
		TGraph2D *tridimensional = new TGraph2D(experiments*experiments);
		tridimensional->SetTitle("Experimental domain; Factor 1; Factor 2; Factor 3"); 
		tridimensional->SetNpy(experiments);
		tridimensional->SetNpx(experiments);
		for (x=0; x<experiments; x++) {
 			tridimensional->SetPoint(x,arr_f1_rs[x],arr_f2_rs[x],arr_f3_rs[x]);
		}
		gStyle->SetPalette(1);
		tridimensional->SetMarkerStyle(20);
		tridimensional->Draw("pcol");
		
	}

	TCanvas *c4 = new TCanvas("c4","Normal probability plot",0,0,600,400); //Normal probability plot
	for(z=0; z<experiments; z++){
		f1[z]=answers(z,0);
		f2[z]=predicted(z,0);	
	}
	TGraph *normal_pp = new TGraph(experiments, f1, f2);
	normal_pp->Draw("ap*");
	TF1 *normal_pp_fit = new TF1("normal_pp_fit","pol 1",0-999,999); //Fitting of the normal probability plot
	normal_pp->SetTitle("Normal Probability Plot; Answers; Predicted");
   	normal_pp->Fit("normal_pp_fit","C");
	Double_t p1 = normal_pp_fit->GetParameter(0); // NPP fit parameters

	return 0;
} 

void which_t(Int_t rep){
	if(rep==1){t=12.706;}
	if(rep==2){t=4.303;}
	if(rep==3){t=3.182;}
	if(rep==4){t=2.776;}
	if(rep==5){t=2.571;}
	if(rep==6){t=2.447;}
	if(rep==7){t=2.365;}
	if(rep==8){t=2.306;}
	if(rep==9){t=2.262;}
	if(rep==10){t=2.228;}

}
