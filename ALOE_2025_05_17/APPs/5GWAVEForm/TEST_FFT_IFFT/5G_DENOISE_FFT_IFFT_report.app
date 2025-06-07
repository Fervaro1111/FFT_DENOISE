object {
	obj_name=REPORT_M251
	exe_name=REPORT_M25
	#R
	inputs {
		name=input_0
		remote_itf=output_2
		remote_obj=DATASOURCESINK
	}
	inputs {
		name=input_1
		remote_itf=output_2
		remote_obj=5G_MAC
	}
	inputs {
		name=input_2
		remote_itf=output_1
		remote_obj=CHAN_NOISE
	}
	inputs {
		name=input_3
		remote_itf=output_1
		remote_obj=5G_UNCRC_PDSCH
	}
}
#########################################################################REPORT_O24
object {
	obj_name=5G_MAC
	exe_name=5G_MAC_LAYER
	#R
	outputs {
		name=output_0
		remote_itf=input_1
		remote_obj=DATASOURCESINK
	}
	outputs {
		name=output_1
		remote_itf=input_7
		remote_obj=5G_RATE_MATCH_PDSCH
	}
	outputs {
		name=output_2
		remote_itf=input_1
		remote_obj=REPORT_M251
	}
	outputs {
		name=output_4
		remote_itf=input_1
		remote_obj=5G_MOD_16QAM
	}
	outputs {
		name=output_6
		remote_itf=input_1
		remote_obj=5G_Code_Segment
	}
	outputs {
		name=output_8
		remote_itf=input_1
		remote_obj=5G_MAPPING
	}
	outputs {
		name=output_10
		remote_itf=input_1
		remote_obj=SCRAMBLING_F_C
	}
	outputs {
		name=output_7
		remote_itf=input_7
		remote_obj=5G_DeCode_Segment
	}
	outputs {
		name=output_11
		remote_itf=input_1
		remote_obj=5G_DERATE_MATCH_PDSCH
	}
	outputs {
		name=output_12
		remote_itf=input_1
		remote_obj=5G_DEMOD_16QAM
	}
	outputs {
		name=output_13
		remote_itf=input_1
		remote_obj=DESCRAMBLING_F_C
	}
	outputs {
		name=output_14
		remote_itf=input_1
		remote_obj=5G_DEMAPPING
	}
}
########################################
########################################
object {
	obj_name=DATASOURCESINK
	exe_name=DATASOURCESINK_REPORT_M25
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=5G_UNCRC_PDSCH
	}
	inputs {
		name=input_1
		remote_itf=output_0
		remote_obj=5G_MAC
	}
	
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=5G_CRC_PDSCH
	}
	outputs {
		name=output_2
		remote_itf=input_0
		remote_obj=REPORT_M251
	}	
}

########################################
########################################
object {
	obj_name=5G_CRC_PDSCH
	exe_name=5G_CRC_PDSCH_REPORT_M25
	#R
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DATASOURCESINK
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=5G_Code_Segment
	}
}
########################################
########################################
object {
	obj_name=5G_Code_Segment
	exe_name=5G_CODE_BLOCK
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=5G_CRC_PDSCH
	}
	inputs {
		name=input_1
		remote_itf=output_6
		remote_obj=5G_MAC
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=LDPC5G_ENCODER0
	}
	outputs {
		name=output_1
		remote_itf=input_1
		remote_obj=LDPC5G_ENCODER0
	}
}
########################################
########################################
object {
	obj_name=LDPC5G_ENCODER0
	exe_name=LDPC5G_ENCODER
	force_pe=1
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=5G_Code_Segment
	}
	inputs {
		name=input_1
		remote_itf=output_1
		remote_obj=5G_Code_Segment
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=5G_RATE_MATCH_PDSCH
	}
	outputs {
		name=output_1
		remote_itf=input_1
		remote_obj=5G_RATE_MATCH_PDSCH
	}
}
########################################
########################################
object {
	obj_name=5G_RATE_MATCH_PDSCH
	exe_name=5G_RATE_MATCH_PDSCH
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=LDPC5G_ENCODER0
	}
	inputs {
		name=input_1
		remote_itf=output_1
		remote_obj=LDPC5G_ENCODER0
	}
	inputs {
		name=input_7
		remote_itf=output_8
		remote_obj=5G_MAC
	}
		
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=SCRAMBLING_F_C
	}
}
########################################
########################################
object {
	obj_name=SCRAMBLING_F_C
	exe_name=SCRAMBLING_F_C
	#R
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=5G_RATE_MATCH_PDSCH
	}
	inputs {
		name=input_1
		remote_itf=output_10
		remote_obj=5G_MAC
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=5G_MOD_16QAM	
	}
}
##################################################################
########################################
object {
	obj_name=5G_MOD_16QAM
	exe_name=5G_MOD_QAM
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=SCRAMBLING_F_C
	}
	inputs {
		name=input_1
		remote_itf=output_4
		remote_obj=5G_MAC
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=5G_MAPPING	
	}
}
###################################################################DEMOD16QAM
object {
	obj_name=5G_MAPPING
	exe_name=MAPPING_V2
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=5G_MOD_16QAM
	}
	inputs {
		name=input_1
		remote_itf=output_8
		remote_obj=5G_MAC
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DENOISE_IFFT
	}

}
###################################################################
###############################################################################
object {
    obj_name  = DENOISE_IFFT
    exe_name  = DENOISE_FFT
    inputs {
      name       = input_0
      remote_itf = output_0
      remote_obj = 5G_MAPPING
    }
    outputs {
      name       = output_0
      remote_itf = input_0
      remote_obj = ADD_CP
    }
}

###############################################################################
################################################################################
object {
	obj_name=ADD_CP
	exe_name=CICLIC_PREFIX
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DENOISE_IFFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=CHAN_NOISE
	}
}
###################################################################
###############################################################################
object {
    obj_name  = CHAN_NOISE
    exe_name  = CHANNEL_NOISE_REPORT_M25
    inputs {
      name       = input_0
      remote_itf = output_0
      remote_obj = ADD_CP
    }
    outputs {
      name       = output_0
      remote_itf = input_0
      remote_obj = REMOVE_CP
    }
    outputs {
      name       = output_1
      remote_itf = input_2
      remote_obj = REPORT_M251
    }
}

###############################################################################
################################################################################
object {
	obj_name=REMOVE_CP
	exe_name=CICLIC_PREFIX
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=CHAN_NOISE
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DENOISE_FFT	
	}
}
###################################################################DEMOD16QA
###############################################################################
object {
    obj_name  = DENOISE_FFT
    exe_name  = DENOISE_FFT
    inputs {
      name       = input_0
      remote_itf = output_0
      remote_obj = REMOVE_CP
    }
    outputs {
      name       = output_0
      remote_itf = input_0
      remote_obj = 5G_DEMAPPING
    }
}

###############################################################################
###################################################################DEMOD16QAM
object {
	obj_name=5G_DEMAPPING
	exe_name=MAPPING_V2
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DENOISE_FFT
	}
	inputs {
		name=input_1
		remote_itf=output_14
		remote_obj=5G_MAC
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=5G_DEMOD_16QAM
	}
}
###################################################################DEMOD16QA
object {
	obj_name=5G_DEMOD_16QAM
	exe_name=5G_MOD_QAM
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=5G_DEMAPPING
	}
	inputs {
		name=input_1
		remote_itf=output_12
		remote_obj=5G_MAC
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DESCRAMBLING_F_C
	}
}
########################################
########################################
object {
	obj_name=DESCRAMBLING_F_C
	exe_name=SCRAMBLING_F_C
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=5G_DEMOD_16QAM
	}
	inputs {
		name=input_1
		remote_itf=output_13
		remote_obj=5G_MAC
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=5G_DERATE_MATCH_PDSCH	
	}
}
##################################################################
########################################
object {
	obj_name=5G_DERATE_MATCH_PDSCH
	exe_name=5G_RATE_MATCH_PDSCH
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DESCRAMBLING_F_C
	}
	inputs {
		name=input_1
		remote_itf=output_11
		remote_obj=5G_MAC
	}
		
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=LDPC5G_DECODER0
	}
	outputs {
		name=output_1
		remote_itf=input_1
		remote_obj=LDPC5G_DECODER0
	}
}
########################################
########################################
object {
	obj_name=LDPC5G_DECODER0
	exe_name=LDPC5G_DECODER
	force_pe=1
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=5G_DERATE_MATCH_PDSCH
	}
	inputs {
		name=input_1
		remote_itf=output_1
		remote_obj=5G_DERATE_MATCH_PDSCH
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=5G_DeCode_Segment
	}
	outputs {
		name=output_1
		remote_itf=input_1
		remote_obj=5G_DeCode_Segment
	}
}
########################################
########################################
object {
	obj_name=5G_DeCode_Segment
	exe_name=5G_CODE_BLOCK
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=LDPC5G_DECODER0
	}
	inputs {
		name=input_1
		remote_itf=output_1
		remote_obj=LDPC5G_DECODER0
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=5G_UNCRC_PDSCH
	}
	inputs {
		name=input_7
		remote_itf=output_7
		remote_obj=5G_MAC
	}
}
########################################
#######################################################################
object {
	obj_name=5G_UNCRC_PDSCH
	exe_name=5G_CRC_PDSCH_REPORT_M25
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=5G_DeCode_Segment
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DATASOURCESINK 
	}
	outputs {
		name=output_1
		remote_itf=input_3
		remote_obj=REPORT_M251
	}
}


#######################################################################
