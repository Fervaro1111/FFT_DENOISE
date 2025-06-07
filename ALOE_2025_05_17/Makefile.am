
SUBDIRS=aloeSKs/aloeSK15/skel_SK15/lnx_make utils/lnx_make hw_api/lnx sw_daemons/lnx_make cmdman_console sw_api/lnx_make modules/typetools/lnx_make modules/5G_Buffer_PDSCH/lnx_make/ modules/5G_CODE_BLOCK/lnx_make/ modules/5G_CRC_PBCH/lnx_make/ modules/5G_CRC_PDSCH/lnx_make/ modules/5G_CRC_PDSCH_REPORT/lnx_make/ modules/5G_CRC_PDSCH_REPORT_M25/lnx_make/ modules/5G_INTERLEAVER_PDCCH/lnx_make/ modules/5G_MAC_LAYER/lnx_make/ modules/5G_MOD_QAM/lnx_make/ modules/5G_POLAR_DEC/lnx_make/ modules/5G_POLAR_ENC/lnx_make/ modules/5G_RATE_MATCH_PBCH/lnx_make/ modules/5G_RATE_MATCH_PDCCH/lnx_make/ modules/5G_RATE_MATCH_PDSCH/lnx_make/ modules/5G_SYNCHRO/lnx_make/ modules/ADSIMUL/lnx_make/ modules/Buffer2/lnx_make/ modules/BUFFER_PDCCH/lnx_make/ modules/Buffer_Temporal/lnx_make/ modules/CHANNEL_ANALYZER/lnx_make/ modules/channel_noise/lnx_make/ modules/CHANNEL_NOISE_REPORT/lnx_make/ modules/CHANNEL_NOISE_REPORT_M25/lnx_make/ modules/CHANNEL_SUI_REPORT/lnx_make/ modules/CICLIC_PREFIX/lnx_make/ modules/CIRC_BUFFER/lnx_make/ modules/CODE_BLOC/lnx_make/ modules/computeEVM/lnx_make/ modules/CPLX_FILTER/lnx_make/ modules/CPLX_FILTER_REPORT/lnx_make/ modules/CRC/lnx_make/ modules/CRC_PDCCH/lnx_make/ modules/CRC_REPORT/lnx_make/ modules/DAC_JACK0/lnx_make/ modules/DAC_JACK1/lnx_make/ modules/data_source/lnx_make/ modules/data_source_sink/lnx_make/ modules/DATASOURCESINK_REPORT/lnx_make/ modules/DATASOURCESINK_REPORT_M25/lnx_make/ modules/DCI_PARSER/lnx_make/ modules/DDC/lnx_make/ modules/DDC17/lnx_make/ modules/DEMODULATOR/lnx_make/ modules/DENOISE_FFT/lnx_make/ modules/DUC17/lnx_make/ modules/EQUALIZER/lnx_make/ modules/EVM_REPORT/lnx_make/ modules/FFT_IFFT/lnx_make/ modules/GRAPH/lnx_make/ modules/LDPC5G_DECODER/lnx_make/ modules/LDPC5G_ENCODER/lnx_make/ modules/LTEturboCOD/lnx_make/ modules/LTEturboCOD2/lnx_make/ modules/MAC_ANTONI/lnx_make/ modules/MAC_LAYER/lnx_make/ modules/MAC_RX_DL/lnx_make/ modules/MAPPING/lnx_make/ modules/MAPPING_V2/lnx_make/ modules/MAPPING_V2_UL/lnx_make/ modules/MIB_GEN/lnx_make/ modules/MODULATOR/lnx_make/ modules/module_template/lnx_make/ modules/MODULPROVA/lnx_make/ modules/RATE_MATCH/lnx_make/ modules/RATE_MATCHING/lnx_make/ modules/RATE_RECOVER_LDPC/lnx_make/ modules/REAL_FILTER/lnx_make/ modules/RECONCATENATION/lnx_make/ modules/REPORT/lnx_make/ modules/REPORT_M25/lnx_make/ modules/SCRAMBLING/lnx_make/ modules/SCRAMBLING_F_C/lnx_make/ modules/SEPARATE/lnx_make/ modules/SYNCRO/lnx_make/ modules/Transform_Precoding/lnx_make/ modules/UNRATE_MATCH/lnx_make/ modules/UPLINK_DEMAPPING/lnx_make/ modules/UPLINK_EQUALIZER/lnx_make/ modules/UPLINK_MAPPING/lnx_make/ modules/UPLINK_SYNCHRO/lnx_make/ modules/WIMAX_DEC_ALOE16/lnx_make/ modules/WIMAX_ENC_ALOE16/lnx_make/ 

EXTRA_DIST=scripts doc example-repository aloeui matlab aloeui.run

dist_bin_SCRIPTS=scripts/viewlogs.sh scripts/update_modules.pl aloeui.run

dist-hook:
	rm -rf `find $(distdir)/ -name .svn`
	rm -rf `find $(distdir)/ -name .deps`
	rm -rf `find $(distdir)/ -name .libs`
	rm -rf `find $(distdir)/ -name bin_*`
	rm -rf `find $(distdir)/ -name lib_*`
	rm -rf `find $(distdir)/ -name ._*`
	rm -rf `find $(distdir)/ -name .dirstamp`
	
	
	
	
