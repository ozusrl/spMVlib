#include "profiler.h"
#include "svmAnalyzer.h"
#include "method.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include "duffsDeviceCSRDD.hpp"
#include "duffsDeviceCompressed.hpp"
#include "incrementalCSR.hpp"

using namespace thundercat;
using namespace std;
using namespace asmjit;

bool __DEBUG__ = false;
bool DUMP_OBJECT = false;
bool DUMP_MATRIX = false;
bool MATRIX_STATS = false;
unsigned int NUM_OF_THREADS = 1;
int ITERS = -1;
Matrix *csrMatrix;
SpMVMethod *method;
vector<MultByMFun> fptrs;
double *vVector;
double *wVector;


void parseCommandLineArguments(int argc, const char *argv[]);
void setParallelism();
void dumpMatrixIfRequested();
void doSVMAnalysisIfRequested();
void registerLoggersIfRequested();
void generateFunctions();
void dumpObjectIfRequested();
void populateInputOutputVectors();
void benchmark();
void cleanup();

int main(int argc, const char *argv[]) {
  parseCommandLineArguments(argc, argv);
  setParallelism();
  method->init(csrMatrix, NUM_OF_THREADS);
  dumpMatrixIfRequested();
  doSVMAnalysisIfRequested();
  registerLoggersIfRequested();
  generateFunctions();
  populateInputOutputVectors();
  benchmark();
  cleanup();
  return 0;
}

void parseCommandLineArguments(int argc, const char *argv[]) {
  // Usage: thundercat <matrixName> <specializerName> {-debug|-dump_object|-dump_matrix|-num_threads|-matrix_stats}
  // E.g: thundercat matrices/fidap037 CSRbyNZ
  // E.g: thundercat matrices/fidap037 RowPattern -debug
  // E.g: thundercat matrices/fidap037 GenOSKI44 -dump_object
  // E.g: thundercat matrices/fidap037 PlainCSR -num_threads 6
  string genOSKI("GenOSKI");
  string genOSKI33("GenOSKI33");
  string genOSKI44("GenOSKI44");
  string genOSKI55("GenOSKI55");
  string csrByNZ("CSRbyNZ");
  string unfolding("Unfolding");
  string rowPattern("RowPattern");
  string unrollingWithGOTO("UnrollingWithGOTO");
  string csrWithGOTO("CSRWithGOTO");
  string csrLenWithGOTO("CSRLenWithGOTO");
  string mkl("MKL");

  string plainCSR("PlainCSR");
  string plainCSR4("PlainCSR4");
  string plainCSR8("PlainCSR8");
  string plainCSR16("PlainCSR16");
  string plainCSR32("PlainCSR32");

  string rowIncrementalCSR("RowIncrementalCSR");

  string duffsDevice4("DuffsDevice4");
  string duffsDevice8("DuffsDevice8");
  string duffsDevice16("DuffsDevice16");
  string duffsDevice32("DuffsDevice32");

  string duffsDeviceLCSR4("DuffsDeviceLCSR4");
  string duffsDeviceLCSR8("DuffsDeviceLCSR8");
  string duffsDeviceLCSR16("DuffsDeviceLCSR16");
  string duffsDeviceLCSR32("DuffsDeviceLCSR32");

  string duffsDeviceCSRDD4("DuffsDeviceCSRDD4");
  string duffsDeviceCSRDD8("DuffsDeviceCSRDD8");
  string duffsDeviceCSRDD16("DuffsDeviceCSRDD16");
  string duffsDeviceCSRDD32("DuffsDeviceCSRDD32");
  
  string duffsDeviceCompressed4("DuffsDeviceCompressed4");
  string duffsDeviceCompressed8("DuffsDeviceCompressed8");
  string duffsDeviceCompressed16("DuffsDeviceCompressed16");
  string duffsDeviceCompressed32("DuffsDeviceCompressed32");
  
  string debugFlag("-debug");
  string dumpObjFlag("-dump_object");
  string dumpMatrixFlag("-dump_matrix");
  string numThreadsFlag("-num_threads");
  string matrixStatsFlag("-matrix_stats");
  string itersFlag("-iters");
  
  string matrixName(argv[1]);
  csrMatrix = Matrix::readMatrixFromFile(matrixName + ".mtx");
  
  char **argptr = (char**)&argv[2];
  
  // Read the specializer
  if(genOSKI.compare(*argptr) == 0) {
    int b_r = atoi(*(++argptr));
    int b_c = atoi(*(++argptr));
    method = new GenOSKI(b_r, b_c);
  } else if(genOSKI33.compare(*argptr) == 0) {
    method = new GenOSKI(3, 3);
  } else if(genOSKI44.compare(*argptr) == 0) {
    method = new GenOSKI(4, 4);
  } else if(genOSKI55.compare(*argptr) == 0) {
    method = new GenOSKI(5, 5);
  } else if(unfolding.compare(*argptr) == 0) {
    method = new Unfolding();
  } else if(csrByNZ.compare(*argptr) == 0) {
    method = new CSRbyNZ();
  } else if(rowPattern.compare(*argptr) == 0) {
    method = new RowPattern();
  } else if(unrollingWithGOTO.compare(*argptr) == 0) {
    method = new UnrollingWithGOTO();
  } else if(csrWithGOTO.compare(*argptr) == 0) {
    method = new CSRWithGOTO();
  } else if(csrLenWithGOTO.compare(*argptr) == 0) {
    method = new CSRLenWithGOTO();
  } else if(mkl.compare(*argptr) == 0) {
    method = new MKL();
  } else if(plainCSR.compare(*argptr) == 0) {
    method = new PlainCSR();
  } else if(plainCSR4.compare(*argptr) == 0) {
    method = new PlainCSR4();
  } else if(plainCSR8.compare(*argptr) == 0) {
    method = new PlainCSR8();
  } else if(plainCSR16.compare(*argptr) == 0) {
    method = new PlainCSR16();
  } else if(plainCSR32.compare(*argptr) == 0) {
    method = new PlainCSR32();
  } else if(rowIncrementalCSR.compare(*argptr) == 0) {
    method = new RowIncrementalCSR();
  } else if(duffsDevice4.compare(*argptr) == 0) {
    method = new DuffsDevice4();
  } else if(duffsDevice8.compare(*argptr) == 0) {
    method = new DuffsDevice8();
  } else if(duffsDevice16.compare(*argptr) == 0) {
    method = new DuffsDevice16();
  } else if(duffsDevice32.compare(*argptr) == 0) {
    method = new DuffsDevice32();
  } else if(duffsDeviceLCSR4.compare(*argptr) == 0) {
    method = new DuffsDeviceLCSR4();
  } else if(duffsDeviceLCSR8.compare(*argptr) == 0) {
    method = new DuffsDeviceLCSR8();
  } else if(duffsDeviceLCSR16.compare(*argptr) == 0) {
    method = new DuffsDeviceLCSR16();
  } else if(duffsDeviceLCSR32.compare(*argptr) == 0) {
    method = new DuffsDeviceLCSR32();
  } else if(duffsDeviceCSRDD4.compare(*argptr) == 0) {
    method = new DuffsDeviceCSRDD<4>();
  } else if(duffsDeviceCSRDD8.compare(*argptr) == 0) {
    method = new DuffsDeviceCSRDD<8>();
  } else if(duffsDeviceCSRDD16.compare(*argptr) == 0) {
    method = new DuffsDeviceCSRDD<16>();
  } else if(duffsDeviceCSRDD32.compare(*argptr) == 0) {
    method = new DuffsDeviceCSRDD<32>();
  } else if(duffsDeviceCompressed4.compare(*argptr) == 0) {
    method = new DuffsDeviceCompressed<4>();
  } else if(duffsDeviceCompressed8.compare(*argptr) == 0) {
    method = new DuffsDeviceCompressed<8>();
  } else if(duffsDeviceCompressed16.compare(*argptr) == 0) {
    method = new DuffsDeviceCompressed<16>();
  } else if(duffsDeviceCompressed32.compare(*argptr) == 0) {
    method = new DuffsDeviceCompressed<32>();
  } else {
    std::cerr << "Method " << *argptr << " not found.\n";
    exit(1);
  }
  argptr++;
  
  while (argc > argptr - (char**)&argv[0]) { // the optional flag exists
    if (debugFlag.compare(*argptr) == 0)
      __DEBUG__ = true;
    else if (dumpObjFlag.compare(*argptr) == 0)
      DUMP_OBJECT = true;
    else if (dumpMatrixFlag.compare(*argptr) == 0)
      DUMP_MATRIX = true;
    else if (matrixStatsFlag.compare(*argptr) == 0)
      MATRIX_STATS = true;
    else if (numThreadsFlag.compare(*argptr) == 0) {
      NUM_OF_THREADS = atoi(*(++argptr));
      if (NUM_OF_THREADS < 1) {
        std::cerr << "Number of threads must be >= 1.\n";
        exit(1);
      }
    } else if (itersFlag.compare(*argptr) == 0) {
      ITERS = atoi(*(++argptr));
      if (ITERS < 0) {
        std::cerr << "Number of iterations must be >= 0.\n";
        exit(1);
      }
    } else {
      std::cerr << "Unrecognized flag: " << *argptr << "\n";
      exit(1);
    }
    argptr++;
  }
}

void setParallelism() {
#ifdef OPENMP_EXISTS
  omp_set_num_threads(NUM_OF_THREADS);
  int nthreads = -1;
#pragma omp parallel
  {
#pragma omp master
    {
      nthreads = omp_get_num_threads();
    }
  }
  if (!__DEBUG__ && !DUMP_OBJECT)
    cout << "Num threads = " << nthreads << "\n";
#endif
}

void dumpMatrixIfRequested() {
  if (DUMP_MATRIX) {
    Matrix *matrix = method->getMethodSpecificMatrix();
    matrix->print();
    exit(0);
  }
}

void doSVMAnalysisIfRequested() {
  if (MATRIX_STATS) {
    SVMAnalyzer svmAnalyzer(csrMatrix);
    svmAnalyzer.printFeatures();
    exit(0);
  }
}

void registerLoggersIfRequested() {
  if (DUMP_OBJECT && method->isSpecializer()) {
    Specializer *specializer = (Specializer*)method;
    auto codeHolders = specializer->getCodeHolders();
    for (int i = 0; i < codeHolders->size(); i++) {
      auto codeHolder = codeHolders->at(i);
      FileLogger *logger = new FileLogger();
      //codeLoggers.push_back(logger);
      std::string fileName("generated_");
      fileName.append(std::to_string(i));
      logger->setStream(fopen(fileName.c_str(), "w"));
      logger->addOptions(Logger::kOptionBinaryForm);
      codeHolder->setLogger(logger);
    }
  }
}

void generateFunctions() {
  Profiler::recordTime("generateFunctions", []() {
    Profiler::recordTime("processMatrix", []() {
      method->processMatrix();
    });
    Profiler::recordTime("emitCode", []() {
      method->emitCode();
    });
  });
}

void populateInputOutputVectors() {
  unsigned long n = csrMatrix->n;
  unsigned long m = csrMatrix->m;
  unsigned long nz = csrMatrix->nz;
  vVector = new double[m];
  wVector = new double[n];
  for(int i = 0; i < m; ++i) {
    vVector[i] = i + 1;
  }
  for(int i = 0; i < n; ++i) {
    wVector[i] = i + 1;
  }
}

void setNumIterations() {
  if (__DEBUG__)
    ITERS = 1;
  if (DUMP_OBJECT)
    ITERS = 0;
  if (ITERS < 0) {
    unsigned long nz = csrMatrix->nz;

    if (nz < 5000) {
      ITERS = 500000;
    } else if (nz < 10000) {
      ITERS = 200000;
    } else if (nz < 50000) {
      ITERS = 100000;
    } else if (nz < 100000) {
      ITERS = 50000;
    } else if (nz < 200000) {
      ITERS = 10000;
    } else if (nz < 1000000) {
      ITERS = 5000;
    } else if (nz < 2000000) {
      ITERS = 1000;
    } else if (nz < 3000000) {
      ITERS = 500;
    } else if (nz < 5000000) {
      ITERS = 200;
    } else if (nz < 8000000) {
      ITERS = 100;
    } else if (nz < 12000000) {
      ITERS = 50;
    } else if (nz < 40000000) {
      ITERS = 20;
    } else {
      ITERS = 10;
    }
  }
}

void benchmark() {
  setNumIterations();
  unsigned long n = csrMatrix->n;
  
  if (__DEBUG__) {
    method->spmv(vVector, wVector);
    for(int i = 0; i < n; ++i)
      printf("%g\n", wVector[i]);
  } else {
    // Warmup
    for (unsigned i = 0; i < std::min(3, ITERS); i++) {
      method->spmv(vVector, wVector);
    }
  
    Profiler::recordTime("multByM", []() {
      for (unsigned i=0; i < ITERS; i++) {
        method->spmv(vVector, wVector);
      }
    });

    Profiler::print(ITERS);
  }
}

void cleanup() {
  /* Normally, a cleanup as follows is the client's responsibility.
     Because we're aliasing some pointers in the returned matrices
     for Unfolding, MKL, and PlainCSR, these deallocations would cause
     double free's. Can be fixed by using shared pointers.
   
   delete csrMatrix;
   delete matrix;
   delete[] v;
   delete[] w;
   delete method;
  */
}
