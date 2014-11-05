#include "fft.h"
//////BEGIN TWIDDLES ////////

void step1(TYPE work_x[], TYPE work_y[], TYPE DATA_x[], 
  TYPE DATA_y[], TYPE data_x[], TYPE data_y[ ], TYPE smem[],
  int reversed[],
  float sin_64[448],
  float cos_64[448],
  float sin_512[448],
  float cos_512[448]
  )
{
	int tid, hi, lo, i, j, stride;
	stride = THREADS;
	//Do it all at once...
	for(tid = 0; tid < THREADS; tid++){
		//GLOBAL_LOAD...
		for(i = 0; i<8;i++){
			data_x[i] = work_x[i*stride+tid];
			data_y[i] = work_y[i*stride+tid];
		}
		FFT8(data_x, data_y);

		//First Twiddle
		//twiddles8_512(data_x, data_y, tid, 512);
     for(j = 1; j < 8; j++){
            TYPE A_x, A_y, tmp;
            A_x = cos_512[tid*7+j-1];
            A_y = sin_512[tid*7+j-1];
            tmp = data_x[j];
            data_x[j] = cmplx_MUL_x(data_x[j], data_y[j], A_x, A_y);
            data_y[j] = cmplx_MUL_y(tmp, data_y[j], A_x, A_y);
    }

		//save for fence
		for(i = 0; i < 8; i ++){
			DATA_x[tid*8 + i] = data_x[i];
			DATA_y[tid*8 + i] = data_y[i];
		}
	}
}
void step2(TYPE work_x[], TYPE work_y[], TYPE DATA_x[], 
  TYPE DATA_y[], TYPE data_x[], TYPE data_y[ ], TYPE smem[], 
  int reversed[],
  float sin_64[448],
  float cos_64[448],
  float sin_512[448],
  float cos_512[448]
  )
{
	int tid, hi, lo, i, j, stride;
	stride = THREADS;
	for(tid = 0; tid < 64; tid++){
		for(i = 0; i < 8; i ++){
			data_x[i] = DATA_x[tid*8 + i];
			//data_y[i] = DATA_y[tid*8 + i];
		}
		hi = tid>>3;
		lo = tid&7;
		store8(data_x, smem, hi*8+lo, 66, reversed);
	}

}
void step3(TYPE work_x[], TYPE work_y[], TYPE DATA_x[], 
  TYPE DATA_y[], TYPE data_x[], TYPE data_y[ ], TYPE smem[],
  int reversed[],
  float sin_64[448],
  float cos_64[448],
  float sin_512[448],
  float cos_512[448]
  )
{
	int tid, hi, lo, i, j, stride;
	stride = THREADS;
	for(tid = 0; tid < 64; tid++){
		for(i = 0; i < 8; i ++){
			data_x[i] = DATA_x[tid*8 + i];
			//data_y[i] = DATA_y[tid*8 + i];
		}
		hi = tid>>3;
		lo = tid&7;
		load8(data_x, smem, lo*66+hi, 8);
		for(i = 0; i < 8; i ++){
			DATA_x[tid*8 + i] = data_x[i];
			//DATA_y[tid*8 + i] = data_y[i];
		}
	}
}
void step4(TYPE work_x[], TYPE work_y[], TYPE DATA_x[], 
  TYPE DATA_y[], TYPE data_x[], TYPE data_y[ ], TYPE smem[], 
  int reversed[],
  float sin_64[448],
  float cos_64[448],
  float sin_512[448],
  float cos_512[448]
  )
{
	int tid, hi, lo, i, j, stride;
	stride = THREADS;
	for(tid = 0; tid < 64; tid++){
		for(i = 0; i < 8; i ++){
			//data_x[i] = DATA_x[tid*8 + i];
			data_y[i] = DATA_y[tid*8 + i];
		}
		hi = tid>>3;
		lo = tid&7;
		store8(data_y, smem, hi*8+lo, 66, reversed);
	}
}
void step5(TYPE work_x[], TYPE work_y[], TYPE DATA_x[], 
  TYPE DATA_y[], TYPE data_x[], TYPE data_y[ ], TYPE smem[],
  int reversed[],
  float sin_64[448],
  float cos_64[448],
  float sin_512[448],
  float cos_512[448]
  )
{

	int tid, hi, lo, i, j, stride;
	stride = THREADS;
	for(tid = 0; tid < 64; tid++){
		for(i = 0; i < 8; i ++){
			//data_x[i] = DATA_x[tid*8 + i];
			data_y[i] = DATA_y[tid*8 + i];
		}
		hi = tid>>3;
		lo = tid&7;
		load8(data_y, smem, lo*66+hi, 8);
		for(i = 0; i < 8; i ++){
			//DATA_x[tid*8 + i] = data_x[i];
			DATA_y[tid*8 + i] = data_y[i];
		}
	}

}
void step6(TYPE work_x[], TYPE work_y[], TYPE DATA_x[], 
  TYPE DATA_y[], TYPE data_x[], TYPE data_y[ ], TYPE smem[],
  int reversed[],
  float sin_64[448],
  float cos_64[448],
  float sin_512[448],
  float cos_512[448]
  )
{
	int tid, hi, lo, i, j, stride;
	stride = THREADS;

	for(tid = 0; tid < 64; tid++){
		//Reload data post-transpose...
		for(i = 0; i < 8; i ++){
			data_x[i] = DATA_x[tid*8 + i];
			data_y[i] = DATA_y[tid*8 + i];
		}

		//Second FFT8...
		FFT8(data_x, data_y);

		//Calculate hi for second twiddle calculation...
		hi = tid>>3;

		//Second twiddles calc, use hi and 64 stride version as defined in G80/SHOC...
		//twiddles8_64(data_x, data_y, hi, 64);
    loop: for(j = 1; j < 8; j++){
            TYPE A_x, A_y, tmp;
            A_x = cos_64[hi*7+j-1];
            A_y = sin_64[hi*7+j-1];
            tmp = data_x[j];
            data_x[j] = cmplx_M_x(data_x[j], data_y[j], A_x, A_y);
            data_y[j] = cmplx_M_y(tmp, data_y[j], A_x, A_y);
    }

		//Save for final transpose...
		for(i = 0; i < 8; i ++){
			DATA_x[tid*8 + i] = data_x[i];
			DATA_y[tid*8 + i] = data_y[i];
		}
	}

}
void step7(TYPE work_x[], TYPE work_y[], TYPE DATA_x[], 
  TYPE DATA_y[], TYPE data_x[], TYPE data_y[ ], TYPE smem[],
  int reversed[],
  float sin_64[448],
  float cos_64[448],
  float sin_512[448],
  float cos_512[448]
  )
{
	int tid, hi, lo, i, j, stride;
	stride = THREADS;
	//Transpose..
	for(tid = 0; tid < 64; tid++){
		for(i = 0; i < 8; i ++){
			data_x[i] = DATA_x[tid*8 + i];
			//data_y[i] = DATA_y[tid*8 + i];
		}
		hi = tid>>3;
		lo = tid&7;
		store8(data_x, smem, hi*8+lo, 72, reversed);
	}

}
void step8(TYPE work_x[], TYPE work_y[], TYPE DATA_x[], 
  TYPE DATA_y[], TYPE data_x[], TYPE data_y[ ], TYPE smem[],
  int reversed[],
  float sin_64[448],
  float cos_64[448],
  float sin_512[448],
  float cos_512[448]
  )
{
	int tid, hi, lo, i, j, stride;
	stride = THREADS;

	for(tid = 0; tid < 64; tid++){
		for(i = 0; i < 8; i ++){
			data_x[i] = DATA_x[tid*8 + i];
			//data_y[i] = DATA_y[tid*8 + i];
		}
		hi = tid>>3;
		lo = tid&7;
		load8(data_x, smem, hi*72+lo, 8);
		for(i = 0; i < 8; i ++){
			DATA_x[tid*8 + i] = data_x[i];
			//DATA_y[tid*8 + i] = data_y[i];
		}
	}

}
void step9(TYPE work_x[], TYPE work_y[], TYPE DATA_x[], 
  TYPE DATA_y[], TYPE data_x[], TYPE data_y[ ], TYPE smem[],
  int reversed[],
  float sin_64[448],
  float cos_64[448],
  float sin_512[448],
  float cos_512[448]
  )
{

	int tid, hi, lo, i, j, stride;
	stride = THREADS;
	for(tid = 0; tid < 64; tid++){
		for(i = 0; i < 8; i ++){
			//data_x[i] = DATA_x[tid*8 + i];
			data_y[i] = DATA_y[tid*8 + i];
		}
		hi = tid>>3;
		lo = tid&7;
		store8(data_y, smem, hi*8+lo, 72, reversed);
	}

}
void step10(TYPE work_x[], TYPE work_y[], TYPE DATA_x[], 
  TYPE DATA_y[], TYPE data_x[], TYPE data_y[ ], TYPE smem[],
  int reversed[],
  float sin_64[448],
  float cos_64[448],
  float sin_512[448],
  float cos_512[448]
  )
{

	int tid, hi, lo, i, j, stride;
	stride = THREADS;
	for(tid = 0; tid < 64; tid++){
		for(i = 0; i < 8; i ++){
			//data_x[i] = DATA_x[tid*8 + i];
			data_y[i] = DATA_y[tid*8 + i];
		}
		hi = tid>>3;
		lo = tid&7;
		load8(data_y, smem, hi*72+lo, 8);
		for(i = 0; i < 8; i ++){
			//DATA_x[tid*8 + i] = data_x[i];
			DATA_y[tid*8 + i] = data_y[i];
		}
	}
	
}
void step11(TYPE work_x[], TYPE work_y[], TYPE DATA_x[], 
  TYPE DATA_y[], TYPE data_x[], TYPE data_y[ ], TYPE smem[], 
  int reversed[],
  float sin_64[448],
  float cos_64[448],
  float sin_512[448],
  float cos_512[448]
  )
{

	int tid, hi, lo, i, j, stride;
	stride = THREADS;
	for(tid = 0; tid < 64; tid++){
		//Load post-trans
		for(i = 0; i < 8; i ++){
			data_x[i] = DATA_x[tid*8 + i];
			data_y[i] = DATA_y[tid*8 + i];
		}

		//Final 8pt FFT...
		FFT8(data_x, data_y);

		//Global store
		for(i=0; i<8;i++){
			//work[i*stride+tid] = data[i];
			work_x[i*stride+tid] = data_x[reversed[i]];
			work_y[i*stride+tid] = data_y[reversed[i]];
		}
	}
}
void fft1D_512(TYPE work_x[512], TYPE work_y[512], 
	TYPE DATA_x[THREADS*8],
	TYPE DATA_y[THREADS*8],
	TYPE data_x[ 8 ],
	TYPE data_y[ 8 ],
	TYPE smem[8*8*9],
  int reversed[8],
  float sin_64[448],
  float cos_64[448],
  float sin_512[448],
  float cos_512[448]
)
{
	int tid, hi, lo, i, j, stride;
	stride = THREADS;
	

  step1(work_x, work_y, DATA_x, DATA_y, data_x, data_y, smem, reversed, sin_64, cos_64, sin_512, cos_512);
  step2(work_x, work_y, DATA_x, DATA_y, data_x, data_y, smem, reversed, sin_64, cos_64, sin_512, cos_512);
  step3(work_x, work_y, DATA_x, DATA_y, data_x, data_y, smem, reversed, sin_64, cos_64, sin_512, cos_512);
  step4(work_x, work_y, DATA_x, DATA_y, data_x, data_y, smem, reversed, sin_64, cos_64, sin_512, cos_512);
  step5(work_x, work_y, DATA_x, DATA_y, data_x, data_y, smem, reversed, sin_64, cos_64, sin_512, cos_512);
  step6(work_x, work_y, DATA_x, DATA_y, data_x, data_y, smem, reversed, sin_64, cos_64, sin_512, cos_512);
  step7(work_x, work_y, DATA_x, DATA_y, data_x, data_y, smem, reversed, sin_64, cos_64, sin_512, cos_512);
  step8(work_x, work_y, DATA_x, DATA_y, data_x, data_y, smem, reversed, sin_64, cos_64, sin_512, cos_512);
  step9(work_x, work_y, DATA_x, DATA_y, data_x, data_y, smem, reversed, sin_64, cos_64, sin_512, cos_512);
  step10(work_x, work_y, DATA_x, DATA_y, data_x, data_y, smem, reversed, sin_64, cos_64, sin_512, cos_512);
  step11(work_x, work_y, DATA_x, DATA_y, data_x, data_y, smem, reversed, sin_64, cos_64, sin_512, cos_512);
}
int main(){
	TYPE a_x[512];
	TYPE a_y[512];
	int i;

	for( i = 0; i < 512; i++){
		a_x[i] = i;
		a_y[i] = 0.0;
	}
  float sin_64[448] = {-0.000000,	-0.000000,	-0.000000,	-0.000000,	-0.000000,	-0.000000,	-0.000000,
    -0.382683,	-0.195090,	-0.555570,	-0.098017,	-0.471397,	-0.290285,	-0.634393,
    -0.707107,	-0.382683,	-0.923880,	-0.195090,	-0.831470,	-0.555570,	-0.980785,
    -0.923880,	-0.555570,	-0.980785,	-0.290285,	-0.995185,	-0.773010,	-0.881921,
    -1.000000,	-0.707107,	-0.707107,	-0.382683,	-0.923880,	-0.923880,	-0.382683,
    -0.923880,	-0.831470,	-0.195090,	-0.471397,	-0.634393,	-0.995185,	0.290285,
    -0.707107,	-0.923880,	0.382683,	-0.555570,	-0.195090,	-0.980785,	0.831470,
    -0.382683,	-0.980785,	0.831470,	-0.634393,	0.290285,	-0.881921,	0.995185,
    -0.000000,	-1.000000,	1.000000,	-0.707107,	0.707107,	-0.707107,	0.707107,
    0.382683,	-0.980785,	0.831470,	-0.773010,	0.956940,	-0.471397,	0.098017,
    0.707107,	-0.923880,	0.382683,	-0.831470,	0.980785,	-0.195090,	-0.555570,
    0.923880,	-0.831470,	-0.195090,	-0.881921,	0.773010,	0.098017,	-0.956940,
    1.000000,	-0.707107,	-0.707107,	-0.923880,	0.382683,	0.382683,	-0.923880,
    0.923880,	-0.555570,	-0.980785,	-0.956940,	-0.098017,	0.634393,	-0.471397,
    0.707107,	-0.382683,	-0.923880,	-0.980785,	-0.555570,	0.831470,	0.195090,
    0.382683,	-0.195090,	-0.555570,	-0.995185,	-0.881921,	0.956940,	0.773010,
    0.000000,	-0.000000,	-0.000000,	-1.000000,	-1.000000,	1.000000,	1.000000,
    -0.382683,	0.195090,	0.555570,	-0.995185,	-0.881921,	0.956940,	0.773010,
    -0.707107,	0.382683,	0.923880,	-0.980785,	-0.555570,	0.831470,	0.195090,
    -0.923880,	0.555570,	0.980785,	-0.956940,	-0.098017,	0.634393,	-0.471397,
    -1.000000,	0.707107,	0.707107,	-0.923880,	0.382683,	0.382683,	-0.923880,
    -0.923880,	0.831470,	0.195090,	-0.881921,	0.773010,	0.098017,	-0.956940,
    -0.707107,	0.923880,	-0.382683,	-0.831470,	0.980785,	-0.195090,	-0.555570,
    -0.382683,	0.980785,	-0.831470,	-0.773010,	0.956940,	-0.471397,	0.098017,
    -0.000000,	1.000000,	-1.000000,	-0.707107,	0.707107,	-0.707107,	0.707107,
    0.382683,	0.980785,	-0.831470,	-0.634393,	0.290285,	-0.881921,	0.995185,
    0.707107,	0.923880,	-0.382683,	-0.555570,	-0.195090,	-0.980785,	0.831470,
    0.923880,	0.831470,	0.195090,	-0.471397,	-0.634393,	-0.995185,	0.290285,
    1.000000,	0.707107,	0.707107,	-0.382683,	-0.923880,	-0.923880,	-0.382683,
    0.923880,	0.555570,	0.980785,	-0.290285,	-0.995185,	-0.773010,	-0.881921,
    0.707107,	0.382683,	0.923880,	-0.195090,	-0.831470,	-0.555570,	-0.980785,
    0.382683,	0.195090,	0.555570,	-0.098017,	-0.471397,	-0.290285,	-0.634393,
    0.000000,	0.000000,	0.000000,	-0.000000,	-0.000000,	-0.000000,	-0.000000,
    -0.382683,	-0.195090,	-0.555570,	0.098017,	0.471397,	0.290285,	0.634393,
    -0.707107,	-0.382683,	-0.923880,	0.195090,	0.831470,	0.555570,	0.980785,
    -0.923880,	-0.555570,	-0.980785,	0.290285,	0.995185,	0.773010,	0.881921,
    -1.000000,	-0.707107,	-0.707107,	0.382683,	0.923880,	0.923880,	0.382683,
    -0.923880,	-0.831470,	-0.195090,	0.471397,	0.634393,	0.995185,	-0.290285,
    -0.707107,	-0.923880,	0.382683,	0.555570,	0.195090,	0.980785,	-0.831470,
    -0.382683,	-0.980785,	0.831470,	0.634393,	-0.290285,	0.881921,	-0.995185,
    -0.000000,	-1.000000,	1.000000,	0.707107,	-0.707107,	0.707107,	-0.707107,
    0.382683,	-0.980785,	0.831470,	0.773010,	-0.956940,	0.471397,	-0.098017,
    0.707107,	-0.923880,	0.382683,	0.831470,	-0.980785,	0.195090,	0.555570,
    0.923880,	-0.831470,	-0.195090,	0.881921,	-0.773010,	-0.098017,	0.956940,
    1.000000,	-0.707107,	-0.707107,	0.923880,	-0.382683,	-0.382683,	0.923880,
    0.923880,	-0.555570,	-0.980785,	0.956940,	0.098017,	-0.634393,	0.471397,
    0.707107,	-0.382683,	-0.923880,	0.980785,	0.555570,	-0.831470,	-0.195090,
    0.382683,	-0.195090,	-0.555570,	0.995185,	0.881921,	-0.956940,	-0.773010,
    0.000000,	-0.000000,	-0.000000,	1.000000,	1.000000,	-1.000000,	-1.000000,
    -0.382683,	0.195090,	0.555570,	0.995185,	0.881921,	-0.956940,	-0.773010,
    -0.707107,	0.382683,	0.923880,	0.980785,	0.555570,	-0.831470,	-0.195090,
    -0.923880,	0.555570,	0.980785,	0.956940,	0.098017,	-0.634393,	0.471397,
    -1.000000,	0.707107,	0.707107,	0.923880,	-0.382683,	-0.382683,	0.923880,
    -0.923880,	0.831470,	0.195090,	0.881921,	-0.773010,	-0.098017,	0.956940,
    -0.707107,	0.923880,	-0.382683,	0.831470,	-0.980785,	0.195090,	0.555570,
    -0.382683,	0.980785,	-0.831470,	0.773010,	-0.956940,	0.471397,	-0.098017,
    -0.000000,	1.000000,	-1.000000,	0.707107,	-0.707107,	0.707107,	-0.707107,
    0.382683,	0.980785,	-0.831470,	0.634393,	-0.290285,	0.881921,	-0.995185,
    0.707107,	0.923880,	-0.382683,	0.555570,	0.195090,	0.980785,	-0.831470,
    0.923880,	0.831470,	0.195090,	0.471397,	0.634393,	0.995185,	-0.290285,
    1.000000,	0.707107,	0.707107,	0.382683,	0.923880,	0.923880,	0.382683,
    0.923880,	0.555570,	0.980785,	0.290285,	0.995185,	0.773010,	0.881921,
    0.707107,	0.382683,	0.923880,	0.195090,	0.831470,	0.555570,	0.980785,
    0.382683,	0.195090,	0.555570,	0.098017,	0.471397,	0.290285,	0.634393};

  float sin_512[448] = {-0.000000,	-0.000000,	-0.000000,	-0.000000,	-0.000000,	-0.000000,	-0.000000,
    -0.049068,	-0.024541,	-0.073565,	-0.012272,	-0.061321,	-0.036807,	-0.085797,
    -0.098017,	-0.049068,	-0.146730,	-0.024541,	-0.122411,	-0.073565,	-0.170962,
    -0.146730,	-0.073565,	-0.219101,	-0.036807,	-0.183040,	-0.110222,	-0.254866,
    -0.195090,	-0.098017,	-0.290285,	-0.049068,	-0.242980,	-0.146730,	-0.336890,
    -0.242980,	-0.122411,	-0.359895,	-0.061321,	-0.302006,	-0.183040,	-0.416430,
    -0.290285,	-0.146730,	-0.427555,	-0.073565,	-0.359895,	-0.219101,	-0.492898,
    -0.336890,	-0.170962,	-0.492898,	-0.085797,	-0.416430,	-0.254866,	-0.565732,
    -0.382683,	-0.195090,	-0.555570,	-0.098017,	-0.471397,	-0.290285,	-0.634393,
    -0.427555,	-0.219101,	-0.615232,	-0.110222,	-0.524590,	-0.325310,	-0.698376,
    -0.471397,	-0.242980,	-0.671559,	-0.122411,	-0.575808,	-0.359895,	-0.757209,
    -0.514103,	-0.266713,	-0.724247,	-0.134581,	-0.624859,	-0.393992,	-0.810457,
    -0.555570,	-0.290285,	-0.773010,	-0.146730,	-0.671559,	-0.427555,	-0.857729,
    -0.595699,	-0.313682,	-0.817585,	-0.158858,	-0.715731,	-0.460539,	-0.898674,
    -0.634393,	-0.336890,	-0.857729,	-0.170962,	-0.757209,	-0.492898,	-0.932993,
    -0.671559,	-0.359895,	-0.893224,	-0.183040,	-0.795837,	-0.524590,	-0.960431,
    -0.707107,	-0.382683,	-0.923880,	-0.195090,	-0.831470,	-0.555570,	-0.980785,
    -0.740951,	-0.405241,	-0.949528,	-0.207111,	-0.863973,	-0.585798,	-0.993907,
    -0.773010,	-0.427555,	-0.970031,	-0.219101,	-0.893224,	-0.615232,	-0.999699,
    -0.803208,	-0.449611,	-0.985278,	-0.231058,	-0.919114,	-0.643832,	-0.998118,
    -0.831470,	-0.471397,	-0.995185,	-0.242980,	-0.941544,	-0.671559,	-0.989177,
    -0.857729,	-0.492898,	-0.999699,	-0.254866,	-0.960431,	-0.698376,	-0.972940,
    -0.881921,	-0.514103,	-0.998795,	-0.266713,	-0.975702,	-0.724247,	-0.949528,
    -0.903989,	-0.534998,	-0.992480,	-0.278520,	-0.987301,	-0.749136,	-0.919114,
    -0.923880,	-0.555570,	-0.980785,	-0.290285,	-0.995185,	-0.773010,	-0.881921,
    -0.941544,	-0.575808,	-0.963776,	-0.302006,	-0.999322,	-0.795837,	-0.838225,
    -0.956940,	-0.595699,	-0.941544,	-0.313682,	-0.999699,	-0.817585,	-0.788346,
    -0.970031,	-0.615232,	-0.914210,	-0.325310,	-0.996313,	-0.838225,	-0.732654,
    -0.980785,	-0.634393,	-0.881921,	-0.336890,	-0.989177,	-0.857729,	-0.671559,
    -0.989177,	-0.653173,	-0.844854,	-0.348419,	-0.978317,	-0.876070,	-0.605511,
    -0.995185,	-0.671559,	-0.803208,	-0.359895,	-0.963776,	-0.893224,	-0.534998,
    -0.998795,	-0.689541,	-0.757209,	-0.371317,	-0.945607,	-0.909168,	-0.460539,
    -1.000000,	-0.707107,	-0.707107,	-0.382683,	-0.923880,	-0.923880,	-0.382683,
    -0.998795,	-0.724247,	-0.653173,	-0.393992,	-0.898674,	-0.937339,	-0.302006,
    -0.995185,	-0.740951,	-0.595699,	-0.405241,	-0.870087,	-0.949528,	-0.219101,
    -0.989177,	-0.757209,	-0.534998,	-0.416430,	-0.838225,	-0.960431,	-0.134581,
    -0.980785,	-0.773010,	-0.471397,	-0.427555,	-0.803208,	-0.970031,	-0.049068,
    -0.970031,	-0.788346,	-0.405241,	-0.438616,	-0.765167,	-0.978317,	0.036807,
    -0.956940,	-0.803208,	-0.336890,	-0.449611,	-0.724247,	-0.985278,	0.122411,
    -0.941544,	-0.817585,	-0.266713,	-0.460539,	-0.680601,	-0.990903,	0.207111,
    -0.923880,	-0.831470,	-0.195090,	-0.471397,	-0.634393,	-0.995185,	0.290285,
    -0.903989,	-0.844854,	-0.122411,	-0.482184,	-0.585798,	-0.998118,	0.371317,
    -0.881921,	-0.857729,	-0.049068,	-0.492898,	-0.534998,	-0.999699,	0.449611,
    -0.857729,	-0.870087,	0.024541,	-0.503538,	-0.482184,	-0.999925,	0.524590,
    -0.831470,	-0.881921,	0.098017,	-0.514103,	-0.427555,	-0.998795,	0.595699,
    -0.803208,	-0.893224,	0.170962,	-0.524590,	-0.371317,	-0.996313,	0.662416,
    -0.773010,	-0.903989,	0.242980,	-0.534998,	-0.313682,	-0.992480,	0.724247,
    -0.740951,	-0.914210,	0.313682,	-0.545325,	-0.254866,	-0.987301,	0.780737,
    -0.707107,	-0.923880,	0.382683,	-0.555570,	-0.195090,	-0.980785,	0.831470,
    -0.671559,	-0.932993,	0.449611,	-0.565732,	-0.134581,	-0.972940,	0.876070,
    -0.634393,	-0.941544,	0.514103,	-0.575808,	-0.073565,	-0.963776,	0.914210,
    -0.595699,	-0.949528,	0.575808,	-0.585798,	-0.012272,	-0.953306,	0.945607,
    -0.555570,	-0.956940,	0.634393,	-0.595699,	0.049068,	-0.941544,	0.970031,
    -0.514103,	-0.963776,	0.689541,	-0.605511,	0.110222,	-0.928506,	0.987301,
    -0.471397,	-0.970031,	0.740951,	-0.615232,	0.170962,	-0.914210,	0.997290,
    -0.427555,	-0.975702,	0.788346,	-0.624859,	0.231058,	-0.898674,	0.999925,
    -0.382683,	-0.980785,	0.831470,	-0.634393,	0.290285,	-0.881921,	0.995185,
    -0.336890,	-0.985278,	0.870087,	-0.643832,	0.348419,	-0.863973,	0.983105,
    -0.290285,	-0.989177,	0.903989,	-0.653173,	0.405241,	-0.844854,	0.963776,
    -0.242980,	-0.992480,	0.932993,	-0.662416,	0.460539,	-0.824589,	0.937339,
    -0.195090,	-0.995185,	0.956940,	-0.671559,	0.514103,	-0.803208,	0.903989,
    -0.146730,	-0.997290,	0.975702,	-0.680601,	0.565732,	-0.780737,	0.863973,
    -0.098017,	-0.998795,	0.989177,	-0.689541,	0.615232,	-0.757209,	0.817585,
    -0.049068,	-0.999699,	0.997290,	-0.698376,	0.662416,	-0.732654,	0.765167};

  float cos_64[448] = {1.000000,	1.000000,	1.000000,	1.000000,	1.000000,	1.000000,	1.000000,
    0.923880,	0.980785,	0.831470,	0.995185,	0.881921,	0.956940,	0.773010,
    0.707107,	0.923880,	0.382683,	0.980785,	0.555570,	0.831470,	0.195090,
    0.382683,	0.831470,	-0.195090,	0.956940,	0.098017,	0.634393,	-0.471397,
    0.000000,	0.707107,	-0.707107,	0.923880,	-0.382683,	0.382683,	-0.923880,
    -0.382683,	0.555570,	-0.980785,	0.881921,	-0.773010,	0.098017,	-0.956940,
    -0.707107,	0.382683,	-0.923880,	0.831470,	-0.980785,	-0.195090,	-0.555570,
    -0.923880,	0.195090,	-0.555570,	0.773010,	-0.956940,	-0.471397,	0.098017,
    -1.000000,	0.000000,	-0.000000,	0.707107,	-0.707107,	-0.707107,	0.707107,
    -0.923880,	-0.195090,	0.555570,	0.634393,	-0.290285,	-0.881921,	0.995185,
    -0.707107,	-0.382683,	0.923880,	0.555570,	0.195090,	-0.980785,	0.831470,
    -0.382683,	-0.555570,	0.980785,	0.471397,	0.634393,	-0.995185,	0.290285,
    -0.000000,	-0.707107,	0.707107,	0.382683,	0.923880,	-0.923880,	-0.382683,
    0.382683,	-0.831470,	0.195090,	0.290285,	0.995185,	-0.773010,	-0.881921,
    0.707107,	-0.923880,	-0.382683,	0.195090,	0.831470,	-0.555570,	-0.980785,
    0.923880,	-0.980785,	-0.831470,	0.098017,	0.471397,	-0.290285,	-0.634393,
    1.000000,	-1.000000,	-1.000000,	0.000000,	0.000000,	-0.000000,	-0.000000,
    0.923880,	-0.980785,	-0.831470,	-0.098017,	-0.471397,	0.290285,	0.634393,
    0.707107,	-0.923880,	-0.382683,	-0.195090,	-0.831470,	0.555570,	0.980785,
    0.382683,	-0.831470,	0.195090,	-0.290285,	-0.995185,	0.773010,	0.881921,
    0.000000,	-0.707107,	0.707107,	-0.382683,	-0.923880,	0.923880,	0.382683,
    -0.382683,	-0.555570,	0.980785,	-0.471397,	-0.634393,	0.995185,	-0.290285,
    -0.707107,	-0.382683,	0.923880,	-0.555570,	-0.195090,	0.980785,	-0.831470,
    -0.923880,	-0.195090,	0.555570,	-0.634393,	0.290285,	0.881921,	-0.995185,
    -1.000000,	-0.000000,	0.000000,	-0.707107,	0.707107,	0.707107,	-0.707107,
    -0.923880,	0.195090,	-0.555570,	-0.773010,	0.956940,	0.471397,	-0.098017,
    -0.707107,	0.382683,	-0.923880,	-0.831470,	0.980785,	0.195090,	0.555570,
    -0.382683,	0.555570,	-0.980785,	-0.881921,	0.773010,	-0.098017,	0.956940,
    -0.000000,	0.707107,	-0.707107,	-0.923880,	0.382683,	-0.382683,	0.923880,
    0.382683,	0.831470,	-0.195090,	-0.956940,	-0.098017,	-0.634393,	0.471397,
    0.707107,	0.923880,	0.382683,	-0.980785,	-0.555570,	-0.831470,	-0.195090,
    0.923880,	0.980785,	0.831470,	-0.995185,	-0.881921,	-0.956940,	-0.773010,
    1.000000,	1.000000,	1.000000,	-1.000000,	-1.000000,	-1.000000,	-1.000000,
    0.923880,	0.980785,	0.831470,	-0.995185,	-0.881921,	-0.956940,	-0.773010,
    0.707107,	0.923880,	0.382683,	-0.980785,	-0.555570,	-0.831470,	-0.195090,
    0.382683,	0.831470,	-0.195090,	-0.956940,	-0.098017,	-0.634393,	0.471397,
    0.000000,	0.707107,	-0.707107,	-0.923880,	0.382683,	-0.382683,	0.923880,
    -0.382683,	0.555570,	-0.980785,	-0.881921,	0.773010,	-0.098017,	0.956940,
    -0.707107,	0.382683,	-0.923880,	-0.831470,	0.980785,	0.195090,	0.555570,
    -0.923880,	0.195090,	-0.555570,	-0.773010,	0.956940,	0.471397,	-0.098017,
    -1.000000,	0.000000,	-0.000000,	-0.707107,	0.707107,	0.707107,	-0.707107,
    -0.923880,	-0.195090,	0.555570,	-0.634393,	0.290285,	0.881921,	-0.995185,
    -0.707107,	-0.382683,	0.923880,	-0.555570,	-0.195090,	0.980785,	-0.831470,
    -0.382683,	-0.555570,	0.980785,	-0.471397,	-0.634393,	0.995185,	-0.290285,
    -0.000000,	-0.707107,	0.707107,	-0.382683,	-0.923880,	0.923880,	0.382683,
    0.382683,	-0.831470,	0.195090,	-0.290285,	-0.995185,	0.773010,	0.881921,
    0.707107,	-0.923880,	-0.382683,	-0.195090,	-0.831470,	0.555570,	0.980785,
    0.923880,	-0.980785,	-0.831470,	-0.098017,	-0.471397,	0.290285,	0.634393,
    1.000000,	-1.000000,	-1.000000,	-0.000000,	-0.000000,	0.000000,	-0.000000,
    0.923880,	-0.980785,	-0.831470,	0.098017,	0.471397,	-0.290285,	-0.634393,
    0.707107,	-0.923880,	-0.382683,	0.195090,	0.831470,	-0.555570,	-0.980785,
    0.382683,	-0.831470,	0.195090,	0.290285,	0.995185,	-0.773010,	-0.881921,
    -0.000000,	-0.707107,	0.707107,	0.382683,	0.923880,	-0.923880,	-0.382683,
    -0.382683,	-0.555570,	0.980785,	0.471397,	0.634393,	-0.995185,	0.290285,
    -0.707107,	-0.382683,	0.923880,	0.555570,	0.195090,	-0.980785,	0.831470,
    -0.923880,	-0.195090,	0.555570,	0.634393,	-0.290285,	-0.881921,	0.995185,
    -1.000000,	-0.000000,	-0.000000,	0.707107,	-0.707107,	-0.707107,	0.707107,
    -0.923880,	0.195090,	-0.555570,	0.773010,	-0.956940,	-0.471397,	0.098017,
    -0.707107,	0.382683,	-0.923880,	0.831470,	-0.980785,	-0.195090,	-0.555570,
    -0.382683,	0.555570,	-0.980785,	0.881921,	-0.773010,	0.098017,	-0.956940,
    -0.000000,	0.707107,	-0.707107,	0.923880,	-0.382683,	0.382683,	-0.923880,
    0.382683,	0.831470,	-0.195090,	0.956940,	0.098017,	0.634393,	-0.471397,
    0.707107,	0.923880,	0.382683,	0.980785,	0.555570,	0.831470,	0.195090,
    0.923880,	0.980785,	0.831470,	0.995185,	0.881921,	0.956940,	0.773010};

  float cos_512[448] = {1.000000,	1.000000,	1.000000,	1.000000,	1.000000,	1.000000,	1.000000,
    0.998795,	0.999699,	0.997290,	0.999925,	0.998118,	0.999322,	0.996313,
    0.995185,	0.998795,	0.989177,	0.999699,	0.992480,	0.997290,	0.985278,
    0.989177,	0.997290,	0.975702,	0.999322,	0.983105,	0.993907,	0.966976,
    0.980785,	0.995185,	0.956940,	0.998795,	0.970031,	0.989177,	0.941544,
    0.970031,	0.992480,	0.932993,	0.998118,	0.953306,	0.983105,	0.909168,
    0.956940,	0.989177,	0.903989,	0.997290,	0.932993,	0.975702,	0.870087,
    0.941544,	0.985278,	0.870087,	0.996313,	0.909168,	0.966976,	0.824589,
    0.923880,	0.980785,	0.831470,	0.995185,	0.881921,	0.956940,	0.773010,
    0.903989,	0.975702,	0.788346,	0.993907,	0.851355,	0.945607,	0.715731,
    0.881921,	0.970031,	0.740951,	0.992480,	0.817585,	0.932993,	0.653173,
    0.857729,	0.963776,	0.689541,	0.990903,	0.780737,	0.919114,	0.585798,
    0.831470,	0.956940,	0.634393,	0.989177,	0.740951,	0.903989,	0.514103,
    0.803208,	0.949528,	0.575808,	0.987301,	0.698376,	0.887640,	0.438616,
    0.773010,	0.941544,	0.514103,	0.985278,	0.653173,	0.870087,	0.359895,
    0.740951,	0.932993,	0.449611,	0.983105,	0.605511,	0.851355,	0.278520,
    0.707107,	0.923880,	0.382683,	0.980785,	0.555570,	0.831470,	0.195090,
    0.671559,	0.914210,	0.313682,	0.978317,	0.503538,	0.810457,	0.110222,
    0.634393,	0.903989,	0.242980,	0.975702,	0.449611,	0.788346,	0.024541,
    0.595699,	0.893224,	0.170962,	0.972940,	0.393992,	0.765167,	-0.061321,
    0.555570,	0.881921,	0.098017,	0.970031,	0.336890,	0.740951,	-0.146730,
    0.514103,	0.870087,	0.024541,	0.966976,	0.278520,	0.715731,	-0.231058,
    0.471397,	0.857729,	-0.049068,	0.963776,	0.219101,	0.689541,	-0.313682,
    0.427555,	0.844854,	-0.122411,	0.960431,	0.158858,	0.662416,	-0.393992,
    0.382683,	0.831470,	-0.195090,	0.956940,	0.098017,	0.634393,	-0.471397,
    0.336890,	0.817585,	-0.266713,	0.953306,	0.036807,	0.605511,	-0.545325,
    0.290285,	0.803208,	-0.336890,	0.949528,	-0.024541,	0.575808,	-0.615232,
    0.242980,	0.788346,	-0.405241,	0.945607,	-0.085797,	0.545325,	-0.680601,
    0.195090,	0.773010,	-0.471397,	0.941544,	-0.146730,	0.514103,	-0.740951,
    0.146730,	0.757209,	-0.534998,	0.937339,	-0.207111,	0.482184,	-0.795837,
    0.098017,	0.740951,	-0.595699,	0.932993,	-0.266713,	0.449611,	-0.844854,
    0.049068,	0.724247,	-0.653173,	0.928506,	-0.325310,	0.416430,	-0.887640,
    0.000000,	0.707107,	-0.707107,	0.923880,	-0.382683,	0.382683,	-0.923880,
    -0.049068,	0.689541,	-0.757209,	0.919114,	-0.438616,	0.348419,	-0.953306,
    -0.098017,	0.671559,	-0.803208,	0.914210,	-0.492898,	0.313682,	-0.975702,
    -0.146730,	0.653173,	-0.844854,	0.909168,	-0.545325,	0.278520,	-0.990903,
    -0.195090,	0.634393,	-0.881921,	0.903989,	-0.595699,	0.242980,	-0.998795,
    -0.242980,	0.615232,	-0.914210,	0.898674,	-0.643832,	0.207111,	-0.999322,
    -0.290285,	0.595699,	-0.941544,	0.893224,	-0.689541,	0.170962,	-0.992480,
    -0.336890,	0.575808,	-0.963776,	0.887640,	-0.732654,	0.134581,	-0.978317,
    -0.382683,	0.555570,	-0.980785,	0.881921,	-0.773010,	0.098017,	-0.956940,
    -0.427555,	0.534998,	-0.992480,	0.876070,	-0.810457,	0.061321,	-0.928506,
    -0.471397,	0.514103,	-0.998795,	0.870087,	-0.844854,	0.024541,	-0.893224,
    -0.514103,	0.492898,	-0.999699,	0.863973,	-0.876070,	-0.012272,	-0.851355,
    -0.555570,	0.471397,	-0.995185,	0.857729,	-0.903989,	-0.049068,	-0.803208,
    -0.595699,	0.449611,	-0.985278,	0.851355,	-0.928506,	-0.085797,	-0.749136,
    -0.634393,	0.427555,	-0.970031,	0.844854,	-0.949528,	-0.122411,	-0.689541,
    -0.671559,	0.405241,	-0.949528,	0.838225,	-0.966976,	-0.158858,	-0.624859,
    -0.707107,	0.382683,	-0.923880,	0.831470,	-0.980785,	-0.195090,	-0.555570,
    -0.740951,	0.359895,	-0.893224,	0.824589,	-0.990903,	-0.231058,	-0.482184,
    -0.773010,	0.336890,	-0.857729,	0.817585,	-0.997290,	-0.266713,	-0.405241,
    -0.803208,	0.313682,	-0.817585,	0.810457,	-0.999925,	-0.302006,	-0.325310,
    -0.831470,	0.290285,	-0.773010,	0.803208,	-0.998795,	-0.336890,	-0.242980,
    -0.857729,	0.266713,	-0.724247,	0.795837,	-0.993907,	-0.371317,	-0.158858,
    -0.881921,	0.242980,	-0.671559,	0.788346,	-0.985278,	-0.405241,	-0.073565,
    -0.903989,	0.219101,	-0.615232,	0.780737,	-0.972940,	-0.438616,	0.012272,
    -0.923880,	0.195090,	-0.555570,	0.773010,	-0.956940,	-0.471397,	0.098017,
    -0.941544,	0.170962,	-0.492898,	0.765167,	-0.937339,	-0.503538,	0.183040,
    -0.956940,	0.146730,	-0.427555,	0.757209,	-0.914210,	-0.534998,	0.266713,
    -0.970031,	0.122411,	-0.359895,	0.749136,	-0.887640,	-0.565732,	0.348419,
    -0.980785,	0.098017,	-0.290285,	0.740951,	-0.857729,	-0.595699,	0.427555,
    -0.989177,	0.073565,	-0.219101,	0.732654,	-0.824589,	-0.624859,	0.503538,
    -0.995185,	0.049068,	-0.146730,	0.724247,	-0.788346,	-0.653173,	0.575808,
    -0.998795,	0.024541,	-0.073565,	0.715731,	-0.749136,	-0.680601,	0.643832};


	TYPE DATA_x[THREADS*8];
	TYPE DATA_y[THREADS*8];
	TYPE data_x[ 8 ];
	TYPE data_y[ 8 ];
	TYPE smem[8*8*9];
  int reversed[8] = {0,4,2,6,1,5,3,7};
	fft1D_512(a_x, a_y, DATA_x, DATA_y, data_x, data_y, smem, reversed, sin_64, sin_512, cos_64, cos_512);

	for( i = 0; i < 2; i++){
		printf("x = %i y = %i \n", a_x[i], a_y[i]);
	}

	return 0;
}
