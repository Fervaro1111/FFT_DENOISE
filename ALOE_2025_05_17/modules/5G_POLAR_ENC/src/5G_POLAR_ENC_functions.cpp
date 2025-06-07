#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <aff3ct.hpp>

#include <fstream>


using namespace aff3ct;
std::unique_ptr<module::Encoder_polar_MK<>>         encoder;
std::vector<int>                                        ref_bits;
std::vector<int>                                        enc_bits;

extern "C" int init_encoder(char *matrix_path, int K, int N) {
	std::string filepath(matrix_path);
	std::vector<int>  vec_reliavility;
	std::vector<int>  vec_reliavility_positions;
	std::vector<bool> frzn_bits (N,false);

	//Open file
	std::ifstream in("Polar_kernels/Reliavility_sequence.txt");
	
	//Get data from file
	
	int i;
	while(in>>i){
		vec_reliavility.push_back(i);
	}
	in.close();
	

	//Get only reliavility sequence needed from the 1024 values sequence
	int l=0;
	for(int q=0;q<vec_reliavility.size();q++){
		if(vec_reliavility[q]<N){
		     vec_reliavility_positions.push_back(vec_reliavility[q]);
		}
	}
		//printf("vec_reliavility.size=%d  ",vec_reliavility.size());

	/*for(int q=0;q<vec_reliavility.size();q++){
		printf("%d  ",vec_reliavility[q]);
	}
	printf("\n");*/

	//Generate the frozen bits pattern
	int frzn_bits_amount=N-K;
	//printf("K=%d,N=%d frz_pos_amount=%d \n",K,N,frzn_bits_amount);
	for(int i=0;i<frzn_bits_amount;i++){
		frzn_bits[vec_reliavility_positions[i]]=true;

	}
	
	//Initialize encoder
	Polar_code* code=new Polar_code(filepath);
	// Init encoder
	encoder = std::unique_ptr<module::Encoder_polar_MK<>>(new module::Encoder_polar_MK<>(K, N,*code,frzn_bits));



/*
	//Initialize encoder
	
	Polar_code* code=new Polar_code(filepath);
	std::vector<bool> frz (8,false);
	// Init encoder
	encoder = std::unique_ptr<module::Encoder_polar_MK<>>(new module::Encoder_polar_MK<>(K, N,*code,frz));
*/

	std::cout << "Should receive " << K << " at the encoder input" << std::endl;
	// Init buffers
	ref_bits = std::vector<int>(K);
	enc_bits = std::vector<int>(N);
	return 0;
}

extern "C" int encode(unsigned char *bytes_in, unsigned char *bits_out, int nin) {
	// Note: Segmentation already done
	// bytes_in must have a size of K/8
	// bits_out must have a size of N
	int j = 0, bp = 7;
	if (nin != ref_bits.size()) {
		return -1;	
	}
	
	for (size_t i = 0; i < ref_bits.size(); i++) {
		ref_bits[i] = (bytes_in[j] >> bp) & 0b1;
		bp--;
		if (bp < 0) {
			j++;
			bp = 7;
		}
	}
	encoder->encode(ref_bits, enc_bits);
	for (size_t i = 0; i < enc_bits.size(); i++) {
		bits_out[i] = enc_bits[i];
	}
	return enc_bits.size();
}

extern "C" int calc_out_bits_max_length(int Carr_agr_lvl,int symb_number_Control){
	int CCE_num=Carr_agr_lvl;
	int REG_per_CCE=6;
	int RE_DMRS_per_CCE=18;
	int RE_in_REG=12;
	int symb_num=symb_number_Control;

	int CCE_total=REG_per_CCE*CCE_num;
	int Available_RE_total=RE_in_REG*symb_num*CCE_total;
	int RE_DMRS_total=RE_DMRS_per_CCE*CCE_num;
	int RE_data=Available_RE_total-RE_DMRS_total;
	int bits_data=2*RE_data; // 2 comes from QPSK is always used in PDCCH
	//printf("E=%d\n",bits_data);
	return bits_data;

}
 
extern "C" int Calc_n(int E,int K){
	float e= (float)E;
	float k=(float)K;
	int n1;
	if( e<=(9/8)*pow(2.0,ceil(log2f(e))-1)  && k/e<9/16){
		n1=ceil(log2f(e))-1;
		//printf("n1 if =%d\n",n1);
	}else{
		n1=ceil(log2f(e));
		//printf("n1 else =%d\n",n1);
	}

	float Rmin=1.0/8.0;
	int n2=ceil(log2f(k/Rmin));
	//printf("n2 =%d\n",n2);
	int nmin=5;
	int nmax=9;
	
	//Calc MIN
	int n_min=1000;
	if(n1<n_min) n_min=n1;
	if(n2<n_min) n_min=n2;
	if(nmax<n_min) n_min=nmax;
	
	//Calc MAX
	if(n_min>nmin)return n_min;
	else return nmin;
 

}

extern "C" int read_file(int N,int K){

	std::vector<int>  vec_reliavility;
	std::vector<int>  vec_reliavility_positions;
	std::vector<bool> frzn_bits (N,false);

	//Open file
	std::ifstream in("Polar_kernels/Reliavility_sequence.txt");
	
	//Get data from file
	
	int i;
	while(in>>i){
		vec_reliavility.push_back(i);
	}
	in.close();
	

	//Get only reliavility sequence needed from the 1024 values sequence
	int l=0;
	for(int q=0;q<vec_reliavility.size();q++){
		if(vec_reliavility[q]<N){
		     vec_reliavility_positions.push_back(vec_reliavility[q]);
		}
	}

	for(int q=0;q<vec_reliavility_positions.size();q++){
		printf("%d  ",vec_reliavility_positions[q]);
	}
	printf("\n");
	//Generate the frozen bits pattern
	int frzn_bits_amount=N-K;
	//printf("K=%d,N=%d frz_pos_amount=%d \n",K,N,frzn_bits_amount);
	for(int i=0;i<frzn_bits_amount;i++){
		frzn_bits[vec_reliavility_positions[i]]=true;

	}

	for(int q=0;q<frzn_bits.size();q++){
		printf("%d  ",(int) frzn_bits[q]);
	}
	

}













