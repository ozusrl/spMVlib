#ifndef _METHOD_H_
#define _METHOD_H_

#include "matrix.h"
#include <unordered_map>
#include <iostream>
#include "asmjit/asmjit.h"

namespace thundercat {
  // multByM(v, w, rows, cols, vals)
  typedef void(*MultByMFun)(double*, double*, int*, int*, double*);
  
  class SpMVMethod {
  public:
    virtual void init(Matrix *csrMatrix, unsigned int numThreads);

    virtual ~SpMVMethod();
    
    virtual bool isSpecializer();
    
    virtual void emitCode();
    
    virtual std::vector<MultByMFun> getMultByMFunctions() = 0;
    
    virtual Matrix* getMethodSpecificMatrix() final;
    
    virtual void processMatrix() final;
  
  protected:
    virtual void analyzeMatrix() = 0;
    virtual void convertMatrix() = 0;
    
    std::vector<MatrixStripeInfo> *stripeInfos;
    Matrix *csrMatrix;
    Matrix *matrix;
    unsigned int numPartitions;
  };
  
  ///
  /// MKL
  ///
  class MKL: public SpMVMethod {
  public:
    virtual void init(Matrix *csrMatrix, unsigned int numThreads) final;
    
    virtual std::vector<MultByMFun> getMultByMFunctions();

  protected:
    virtual void analyzeMatrix();
    virtual void convertMatrix();
  };
  
  ///
  /// PlainCSR
  ///
  class PlainCSR: public SpMVMethod {
  public:
    virtual std::vector<MultByMFun> getMultByMFunctions();
    
  protected:
    virtual void analyzeMatrix();
    virtual void convertMatrix();
  };


  ///
  /// Specializer
  ///
  class Specializer : public SpMVMethod {
  public:
    virtual void init(Matrix *csrMatrix, unsigned int numThreads) final;
    
    virtual bool isSpecializer() final;
    
    virtual void emitCode() final;
  
    virtual std::vector<MultByMFun> getMultByMFunctions() final;

    virtual std::vector<asmjit::CodeHolder*> *getCodeHolders() final;

  protected:
    virtual void emitMultByMFunction(unsigned int index) = 0;
    
    std::vector<asmjit::CodeHolder*> codeHolders;
    
  private:
    void emitConstData();
    
    asmjit::JitRuntime rt;
  };

  ///
  /// CSRbyNZ
  ///
  class RowByNZ {
  public:
    std::vector<int> *getRowIndices();
    void addRowIndex(int index);
    
  private:
    std::vector<int> rowIndices;
  };
  
  // To keep the map keys in ascending order, using the "greater" comparator.
  typedef std::map<unsigned long, RowByNZ, std::greater<unsigned long> > NZtoRowMap;
  
  class CSRbyNZ: public Specializer {
  protected:
    virtual void emitMultByMFunction(unsigned int index) final;
    virtual void analyzeMatrix() final;
    virtual void convertMatrix() final;

  private:
    std::vector<NZtoRowMap> rowByNZLists;
  };
  
  ///
  /// Gen OSKI
  ///
  typedef std::map<unsigned long, std::pair<std::vector<std::pair<int, int> >, std::vector<double> > > GroupByBlockPatternMap;
  
  class GenOSKI: public Specializer {
  public:
    GenOSKI(unsigned int b_r, unsigned int b_c);
    
  protected:
    virtual void emitMultByMFunction(unsigned int index) final;
    virtual void analyzeMatrix() final;
    virtual void convertMatrix() final;

  private:
    unsigned int b_r, b_c;
    std::vector<GroupByBlockPatternMap> groupByBlockPatternMaps;
    std::vector<unsigned int> numBlocks;
  };

//  ///
//  /// Stencil
//  ///
//  class Stencil: public Specializer {
//  public:
//    Stencil(Matrix *csrMatrix);
//
//    virtual void emitMultByMFunction(unsigned int index);
//
//  protected:
//    StencilAnalyzer analyzer;
//  };
//  
//
//  ///
//  /// Unfolding
//  ///
//  class Unfolding: public Specializer {
//  public:
//    Unfolding(Matrix *csrMatrix);
//
//    virtual void emitMultByMFunction(unsigned int index);
//
//  protected:
//    UnfoldingAnalyzer analyzer;
//  };
//
//  ///
//  /// UnrollingWithGOTO
//  ///
//  class UnrollingWithGOTO: public Specializer {
//  public:
//    UnrollingWithGOTO(Matrix *csrMatrix);
//
//    virtual void emitMultByMFunction(unsigned int index);
//
//  protected:
//    UnrollingWithGOTOAnalyzer analyzer;
//  };
//  
//  ///
//  /// CSRWithGOTO
//  ///
//  class CSRWithGOTO: public Specializer {
//  public:
//    CSRWithGOTO(Matrix *csrMatrix);
//    
//    virtual void emitMultByMFunction(unsigned int index);
//
//  protected:
//    CSRWithGOTOAnalyzer analyzer;
//  };
}

#endif
