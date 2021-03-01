
#include <iostream>
#include "htkfile.h"
/*
 *	Swap the byte order 
 */


/* SwapInt32: swap byte order of int32 data value *p */
void SwapInt32(int32_t *p)
{
  char temp,*q;
  
  q = (char*) p;
  temp = *q; *q = *(q+3); *(q+3) = temp;
  temp = *(q+1); *(q+1) = *(q+2); *(q+2) = temp;
}

/* SwapShort: swap byte order of short data value *p */
void SwapShort(short *p)
{
  char temp,*q;
  
  q = (char*) p;
  temp = *q; *q = *(q+1); *(q+1) = temp;
}

bool read_htk(std::string &filename, infra::matrix &features)
{
  FILE *instream;
  if ( (instream=fopen(filename.c_str(), "rb")) == NULL) {
    std::cerr << "Error: unable to open HTK file " << filename << std::endl;
    return false;
  }

  HtkFile my_htk(instream); 

  if (!my_htk.read_header()) {
    std::cerr << "Error: unable to read HTK header of " << filename << std::endl;
    return false;
  }
  
  features.resize(my_htk.nSamples(), my_htk.num_coefs());
  infra::vector data(my_htk.num_coefs());
  int count = 0;
  int startFrame=0;
  while ( my_htk.read_next_vector(data) == my_htk.num_coefs() ) 
  {
    features.row(count++) = data;
  }
  features.resize(count, my_htk.num_coefs());
  fclose(instream);
  return true;
}

bool write_htk(std::string &filename, const infra::matrix &features) {
  int32_t n_row = features.height();
  short n_column = features.width();  
  FILE *outstream;
  if ( (outstream=fopen(filename.c_str(), "wb")) == NULL) {
    std::cerr << "Error: unable to open target file " << filename  
              << " to write data to HTK format" << std::endl;
  }
  HtkFile my_htk(outstream);
  my_htk.header.nSamples = n_row;
  my_htk.header.sampPeriod = 100000;
  my_htk.header.sampSize = n_column * 4;
  my_htk.header.parmKind = 9;
  my_htk.write_header();
  int count = 0;
  for(int i=0; i < features.height(); i++) {
    if (my_htk.write_next_vector(features.row(count)) == my_htk.num_coefs()) {
      count++;
    }
  }
  if (n_row != count) {
    std::cerr << "Error: not all the data written to the HTK file correctly: "
              << filename << std::endl;
  }
  fclose(outstream);
  return true;
}

HtkFile::HtkFile(FILE *_stream)
{
  stream = _stream;
  // check endian type of the machine
  unsigned short s = 0x0001;
  big_endian = ! *(unsigned char*)&s;
}

HtkFile::~HtkFile()
{
  
}

bool HtkFile::read_header()
{
  // This function reads the header into the struct 
	int32_t nSamples;
	int32_t sampPeriod;
	short sampSize;
	short parmKind;
  
  if (fread(&nSamples, 4, 1, stream) != 1)
		return false;
  if (fread(&sampPeriod, 4, 1, stream) != 1)
		return false;
  if (fread(&sampSize, 2, 1, stream) != 1)
		return false;
  if (fread(&parmKind, 2, 1, stream) != 1)
		return false;
  
  if (!big_endian) {
    SwapInt32(&nSamples);
    SwapInt32(&sampPeriod);
    SwapShort(&sampSize);
    SwapShort(&parmKind);
  }
  
  // Should never find EOF here since you would be expecting the data in 
  // the MultiFrame too.
  if (feof(stream) != 0 || ferror(stream) != 0) 
		return false;
  // and sizes of data should be reasonable
	if (sampSize <= 0 || sampSize > 15000 || nSamples <= 0 ||
      sampPeriod <= 0 || sampPeriod > 1000000) {
		return false;
  }
  
	header.nSamples = nSamples;
	header.sampPeriod = sampPeriod;
	header.sampSize = sampSize;
	header.parmKind = parmKind;
	return true;
}

void HtkFile::write_header() 
{
  int32_t nSamples;
  int32_t sampPeriod;
  short sampSize;
  short parmKind;
  
  nSamples = header.nSamples;
  sampPeriod = header.sampPeriod;
  sampSize = header.sampSize;
  parmKind = header.parmKind;

  // swap bytes
  if (!big_endian) {
    SwapInt32(&nSamples);
    SwapInt32(&sampPeriod);
    SwapShort(&sampSize);
    SwapShort(&parmKind);
  }
  
  // This function reads the header into the struct 
  if (fwrite(&nSamples, 4, 1, stream) != 1)
    throw std::bad_alloc();
  if (fwrite(&sampPeriod, 4, 1, stream) != 1)
    throw std::bad_alloc();
  if (fwrite(&sampSize, 2, 1, stream) != 1)
    throw std::bad_alloc();
  if (fwrite(&parmKind, 2, 1, stream) != 1)
    throw std::bad_alloc();
}


size_t HtkFile::num_coefs()
{
  return header.sampSize/sizeof(float);
}

std::string HtkFile::parmKind2str()
{
  std::string str;
	short parmKind = header.parmKind & BASEMASK;
	if (parmKind == 0)
		str = "WAVEFORM";
	else if (parmKind == 1)
		str = "LPC";
	else if (parmKind == 2)
		str = "LPREFC";
	else if (parmKind == 3)
		str = "LPCEPSTRA";
	else if (parmKind == 4)
		str = "LPDELCEP";
	else if (parmKind == 5)
		str = "IREFC";
	else if (parmKind == 6)
		str = "MFCC";
	else if (parmKind == 7)
		str = "FBANK";
	else if (parmKind == 8)
		str = "MELSPEC";
	else if (parmKind == 9)
		str = "USER";
	else if (parmKind == 10)
		str = "DISCRETE";
	else if (parmKind == 11)
		str = "PLP";
	else if (parmKind == 12)
		str = "ANON";
  if (header.parmKind & HASENERGY) str += "_E";
  if (header.parmKind & HASDELTA) str += "_D";
  if (header.parmKind & HASACCS) str += "_N";
  if (header.parmKind & HASTHIRD) str += "_A";
  if (header.parmKind & HASNULLE) str += "_T";
  if (header.parmKind & HASCOMPX) str += "_C";
  if (header.parmKind & HASCRCC) str += "_K";
  if (header.parmKind & HASZEROM) str += "_Z";
  if (header.parmKind & HASZEROC) str += "_O";
  if (header.parmKind & HASVQ) str += "_V";
  return str;
}

void HtkFile::print_header()
{
  std::cout << std::dec << "  nSamples: " << header.nSamples << std::endl;
  std::cout << std::dec << "  sampPeriod: " << (header.sampPeriod/10.0) << " us" << std::endl;
  std::cout << std::dec << "  SampSize: " << header.sampSize << std::endl;
  std::cout << "  parmKind: " << parmKind2str() << std::endl;
  std::cout << "  Num Coefs: " << num_coefs() << std::endl;
  std::cout << "  Machine type: " << (big_endian ? "big" : "little") << " endian." << std::endl;
}


size_t HtkFile::read_next_vector(float *data)
{
  size_t rc =  fread(data,sizeof(float),num_coefs(),stream);
  if (!big_endian) {
    for (size_t i = 0; i < rc; i++) {
      SwapInt32((int32_t*)&(data[i]));
    }
  }
  return rc;
}

size_t HtkFile::read_next_vector(double *data)
{
	float *fdata;
	fdata = new float[num_coefs()];
  
  size_t rc = fread(fdata,sizeof(float), num_coefs(),stream);
  for (size_t i = 0; i < rc; i++) {
    if (!big_endian) {
      SwapInt32((int32_t*)&(fdata[i]));
    }
    data[i] = fdata[i];
  }

	delete [] fdata;

  return rc;
}


size_t HtkFile::read_next_vector(infra::vector &data)
{
  float *fdata;
  fdata = new float[num_coefs()];
  data.resize(num_coefs());
  
  size_t rc = fread(fdata,sizeof(float), num_coefs(),stream);
  for (size_t i = 0; i < rc; i++) {
    if (!big_endian) {
      SwapInt32((int32_t*)&(fdata[i]));
    }
    data[i] = fdata[i];
  }

        delete [] fdata;

  return rc;
}


size_t HtkFile::write_next_vector(float *data)
{
  if (!big_endian) {
    for (size_t i = 0; i < (unsigned int)num_coefs(); i++) {
      SwapInt32((int32_t*)&(data[i]));
    }
  }
  size_t rc =  fwrite(data,sizeof(float),num_coefs(),stream);
  return rc;
}

size_t HtkFile::write_next_vector(double *data)
{
  if (!big_endian) {
    for (size_t i = 0; i < (unsigned int)num_coefs(); i++) {
      SwapInt32((int32_t*)&(data[i]));
    }
  }
  size_t rc =  fwrite(data,sizeof(float),num_coefs(),stream);
  return rc;
}

size_t HtkFile::write_next_vector(const infra::vector &data) {
    float *fdata;
    fdata = new float[num_coefs()];

    if (!big_endian) {
        for (size_t i = 0; i < (unsigned int)num_coefs(); i++) {
           fdata[i] = data[i];
           SwapInt32((int32_t*)&(fdata[i]));
        }
    }
    size_t rc =  fwrite(fdata, sizeof(float), num_coefs(), stream);
    return rc;
}
