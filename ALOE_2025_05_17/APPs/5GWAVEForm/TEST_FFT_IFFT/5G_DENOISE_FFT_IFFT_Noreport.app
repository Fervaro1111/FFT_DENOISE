
#################################################################DATASOURCESINK
object {
	obj_name=data_source
	exe_name=data_source
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=GRAPH_IFFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=GRAPH_Datasource
	}
}
####################################################################GRAPH1
object {
	obj_name=GRAPH_Datasource
	exe_name=GRAPH		
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=data_source
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DENOISE_IFFT
	}	
}
####################################################################
####################################################################IFFT
object {
	obj_name=DENOISE_IFFT
	exe_name=DENOISE_FFT
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=GRAPH_Datasource
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=GRAPH_IFFT	
	}
	
}
###################################################################GRAPH1
object {
	obj_name=GRAPH_IFFT
	exe_name=GRAPH		
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
####################################################################

#################################################CHANNEL_NOISE
object {
	obj_name=CHAN_NOISE
	exe_name=CHANNEL_NOISE_REPORT_M25
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=GRAPH_IFFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=GRAPH_NOISE
	}
}
###################################################################
###################################################################GRAPH2
object {
	obj_name=GRAPH_NOISE
	exe_name=GRAPH		
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
####################################################################
####################################################################FFT
object {
	obj_name=DENOISE_FFT
	exe_name=DENOISE_FFT
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=GRAPH_NOISE
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=GRAPH_FFT
	}
}
###################################################################GRAPH2
object {
	obj_name=GRAPH_FFT
	exe_name=GRAPH		
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DENOISE_FFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=data_source
	}	
}
####################################################################

