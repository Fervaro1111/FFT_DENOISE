#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <aff3ct.hpp>

#include <fstream>

using namespace aff3ct;

std::unique_ptr<module::Encoder_LDPC_from_QC<>>         encoder;
std::unique_ptr<module::Encoder_LDPC_from_QC<>>         encoder2;
std::unique_ptr<module::Decoder_LDPC_BP_flooding_SPA<>> decoder;
std::unique_ptr<module::Decoder_LDPC_BP_flooding_SPA<>> decoder2;
std::vector<float>                                      LLRs;
std::vector<float>                                      LLRs2;
std::vector<int>                                        dec_bits;
std::vector<int>                                        dec_bits2;


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
extern "C" int init_decoder(char *matrix_path, int K, int N, int n_ite, int enable_synd, int synd_depth) {
	std::string filepath(matrix_path);

	// Load parity check matrix from the file
	Sparse_matrix* H = new Sparse_matrix();
	tools::LDPC_matrix_handler::Positions_vector* ibp = new tools::LDPC_matrix_handler::Positions_vector();
	std::vector<bool>* pct = nullptr;
	*H = tools::LDPC_matrix_handler::read(filepath, ibp, pct);
	
	// Init encoder so that we can get the ibp
	encoder = std::unique_ptr<module::Encoder_LDPC_from_QC<>>(new module::Encoder_LDPC_from_QC<>(K, N, *H));
	// Init decoder
	bool es = (enable_synd == 1) ? true : false;
	decoder = std::unique_ptr<module::Decoder_LDPC_BP_flooding_SPA<>>(
		new module::Decoder_LDPC_BP_flooding_SPA<>(K, N, n_ite, *H, encoder->get_info_bits_pos(), es, synd_depth));

	// Init buffers
	std::cout << "Should receive " << N << " at the decoder input" << std::endl;
	LLRs = std::vector<float>(N);
	dec_bits = std::vector<int>(K);
	return 0;
}

extern "C" int init_decoder2(char *matrix_path, int K, int N, int n_ite, int enable_synd, int synd_depth) {
	std::string filepath(matrix_path);

	// Load parity check matrix from the file
	Sparse_matrix* H = new Sparse_matrix();
	tools::LDPC_matrix_handler::Positions_vector* ibp = new tools::LDPC_matrix_handler::Positions_vector();
	std::vector<bool>* pct = nullptr;
	*H = tools::LDPC_matrix_handler::read(filepath, ibp, pct);
	
	// Init encoder so that we can get the ibp
	encoder2 = std::unique_ptr<module::Encoder_LDPC_from_QC<>>(new module::Encoder_LDPC_from_QC<>(K, N, *H));
	// Init decoder
	bool es = (enable_synd == 1) ? true : false;
	decoder2 = std::unique_ptr<module::Decoder_LDPC_BP_flooding_SPA<>>(
		new module::Decoder_LDPC_BP_flooding_SPA<>(K, N, n_ite, *H, encoder2->get_info_bits_pos(), es, synd_depth));

	// Init buffers
	std::cout << "Should receive " << N << " at the decoder input" << std::endl;
	LLRs2 = std::vector<float>(N);
	dec_bits2 = std::vector<int>(K);
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

extern "C" int decode2(float *sbits_in, unsigned char *bytes_out, int nin) {
	// Note: Segmentation already done
	// sbits_in must have a size of N
	// bytes_out must have a size of K/8
	if (nin != LLRs2.size()) {
		return -1;	
	}
	for (size_t i = 0; i < LLRs2.size(); i++) {
		LLRs2[i] = -sbits_in[i]; 	
	}
	decoder2->decode_siho(LLRs2, dec_bits2);
	int j = -1;
	for (size_t i = 0; i < dec_bits2.size(); i++) {
		int b = i % 8;
		if (b == 0) {
			j++;
			bytes_out[j] = 0;
		}
		bytes_out[j] |= dec_bits2[i] << (7 - b);
	}
	return dec_bits2.size()/8;
}


