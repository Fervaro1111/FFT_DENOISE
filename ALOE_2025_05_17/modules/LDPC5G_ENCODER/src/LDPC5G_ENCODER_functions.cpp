#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <aff3ct.hpp>

#include <fstream>

using namespace aff3ct;

std::unique_ptr<module::Encoder_LDPC_from_QC<>>         encoder2;
std::unique_ptr<module::Encoder_LDPC_from_QC<>>         encoder;
std::vector<int>                                        ref_bits;
std::vector<int>                                        enc_bits;
std::vector<int>                                        ref_bits2;
std::vector<int>                                        enc_bits2;

/**
 * @brief Initialize the QC LDPC encoder
 *
 * This function initializes the QC LDPC QC encoder from the aff3ct library.
 * It loads the base graph matrix from the especified file.
 *
 * @param[in] matrix_path The path to the QC file that defines the sparse matrix to use
 * @param[in] K           Number of bits in the data block
 * @param[in] N           Number of bits in the encoded block (data bits + redundancy bits)
 */
extern "C" int init_encoder(char *matrix_path, int K, int N) {
	std::string filepath(matrix_path);

	// Load parity check matrix from the file
	Sparse_matrix* H = new Sparse_matrix();
	tools::LDPC_matrix_handler::Positions_vector* ibp = new tools::LDPC_matrix_handler::Positions_vector();
	std::vector<bool>* pct = nullptr;
	*H = tools::LDPC_matrix_handler::read(filepath, ibp, pct);

	// Init encoder
	encoder = std::unique_ptr<module::Encoder_LDPC_from_QC<>>(new module::Encoder_LDPC_from_QC<>(K, N, *H));
	
	std::cout << "O      Encoder 1 Should receive " << K << " at the encoder input" << std::endl;
	// Init buffers
	ref_bits = std::vector<int>(K);
	enc_bits = std::vector<int>(N);
	return 0;
}

extern "C" int init_encoder2(char *matrix_path, int K, int N) {
	std::string filepath(matrix_path);

	// Load parity check matrix from the file
	Sparse_matrix* H = new Sparse_matrix();
	tools::LDPC_matrix_handler::Positions_vector* ibp = new tools::LDPC_matrix_handler::Positions_vector();
	std::vector<bool>* pct = nullptr;
	*H = tools::LDPC_matrix_handler::read(filepath, ibp, pct);

	// Init encoder
	encoder2 = std::unique_ptr<module::Encoder_LDPC_from_QC<>>(new module::Encoder_LDPC_from_QC<>(K, N, *H));
	
	std::cout << "O      Encoder 2 Should receive " << K << " at the encoder input" << std::endl;
	// Init buffers
	ref_bits2 = std::vector<int>(K);
	enc_bits2 = std::vector<int>(N);
	return 0;
}

/**
 * @brief Encode a code block
 *
 * This function takes in a pointer to an array of bytes of K bits and outputs it to the encoded block of N bits.
 * Each bit at the output is represented as a char.
 *
 * @param[in] bytes_in    Pointer to the buffer of bytes to encode
 * @param[in] bits_out    Pointer to the buffer of encoded bits
 */
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
extern "C" int encode2(unsigned char *bytes_in, unsigned char *bits_out, int nin) {
	// Note: Segmentation already done
	// bytes_in must have a size of K/8
	// bits_out must have a size of N
	int j = 0, bp = 7;
	if (nin != ref_bits2.size()) {
		return -1;	
	}
	
	for (size_t i = 0; i < ref_bits2.size(); i++) {
		ref_bits2[i] = (bytes_in[j] >> bp) & 0b1;
		bp--;
		if (bp < 0) {
			j++;
			bp = 7;
		}
	}
	encoder2->encode(ref_bits2, enc_bits2);
	for (size_t i = 0; i < enc_bits2.size(); i++) {
		bits_out[i] = enc_bits2[i];
	}
	return enc_bits2.size();
}
