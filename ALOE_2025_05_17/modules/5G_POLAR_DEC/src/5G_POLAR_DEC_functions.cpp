#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <aff3ct.hpp>

#include <fstream>

using namespace aff3ct;

//std::unique_ptr<module::Encoder_LDPC_from_QC<>>         encoder;
std::unique_ptr<module::Encoder_polar_MK<>>             encoder;
std::unique_ptr<module::Decoder_polar_MK_SC_naive<>>    decoder;
std::vector<float>                                      LLRs;
std::vector<int>                                        dec_bits;



/**
 * @brief Initialize the QC LDPC decoder
 *
 * This function initializes the QC LDPC QC encoder from the aff3ct library.
 * It loads the base graph matrix from the especified file.
 *
 * @param[in] matrix_path The path to the QC file that defines the sparse matrix to use
 * @param[in] K           Number of bits in the data block
 * @param[in] N           Number of bits in the encoded block (data bits + redundancy bits)
 * @param[in] n_ite       Maximum number of iterations
 * @param[in] enable_synd Enable syndrome detection. Can save processing time since we won't have to
                          compute all iterations every time. 1 -> enable, 0 -> disable
 * @param[in] synd_depth  Number of iterations to process before enabling the syndrome detection
                          In some cases, it can help to avoid false positive detections
 */
extern "C" int init_decoder(char *matrix_path, int K, int N) {
	std::string filepath(matrix_path);
	Polar_code* code=new Polar_code(filepath);
	//std::vector<bool> frz (8,false);

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

	/*for(int q=0;q<vec_reliavility_positions.size();q++){
		printf("%d  ",vec_reliavility_positions[q]);
	}
	printf("\n");*/

	//Generate the frozen bits pattern
	int frzn_bits_amount=N-K;
	//printf("K=%d,N=%d frz_pos_amount=%d \n",K,N,frzn_bits_amount);
	for(int i=0;i<frzn_bits_amount;i++){
		frzn_bits[vec_reliavility_positions[i]]=true;

	}
	

	
	// Init decoder
	//decoder = std::unique_ptr<module::Decoder_polar_MK_SC_naive<>>(new module::Decoder_polar_MK_SC_naive<>(K, N,*code,frz));
	decoder = std::unique_ptr<module::Decoder_polar_MK_SC_naive<>>(new module::Decoder_polar_MK_SC_naive<>(K, N,*code,frzn_bits));
	


	// Init buffers
	std::cout << "Should receive " << N << " at the decoder input" << std::endl;
	LLRs = std::vector<float>(N);
	dec_bits = std::vector<int>(K);
	return 0;
}


/**
 * @brief Decode a code block
 *
 * This function takes in a pointer to an array of N bits and outputs it to the decoded block of N bits.
 * Each bit at the output is represented as a char.
 *
 * @param[in] sbits_in    Pointer to the buffer of soft bits to decode
 * @param[in] bytes_out   Pointer to the buffer of hard bits decoded into bytes
 */
extern "C" int decode(float *sbits_in, unsigned char *bytes_out, int nin) {
	// Note: Segmentation already done
	// sbits_in must have a size of N
	// bytes_out must have a size of K/8
	if (nin != LLRs.size()) {
		return -1;	
	}
	for (size_t i = 0; i < LLRs.size(); i++) {
		LLRs[i] = -sbits_in[i]; 	
	}
	decoder->decode_siho(LLRs, dec_bits);
	int j = -1;
	for (size_t i = 0; i < dec_bits.size(); i++) {
		int b = i % 8;
		if (b == 0) {
			j++;
			bytes_out[j] = 0;
		}
		bytes_out[j] |= dec_bits[i] << (7 - b);
	}
	return dec_bits.size()/8;
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
	printf("n2 =%d\n",n2);
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


