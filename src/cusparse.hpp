#ifndef _THUNDERCAT_CUSPARSE_HPP
#define _THUNDERCAT_CUSPARSE_HPP


#include "method.h"

namespace thundercat {

  class CusparseAdaptor {
  public:
      ~CusparseAdaptor();
      void init();
      void preprocess(int nnz, int m, int n, int *rowPtr, int *colIdx, double *values);
      void spmv(double *v, double *w);
  };

  CusparseAdaptor* newCusparseAdaptor();
  void deleteCusparseAdaptor(CusparseAdaptor* adaptor);

  class Cusparse : public SpmvMethod {
   public:

      virtual void init(unsigned int numThreads);
      virtual ~Cusparse();
      virtual void preprocess(MMMatrix<VALUE_TYPE> &matrix);
      virtual void spmv(double* __restrict v, double* __restrict w);

      static const std::string name;
  private:
      std::unique_ptr<CSRMatrix<VALUE_TYPE>> csrMatrix;
      CusparseAdaptor* adaptor;
  };

}
#endif //_THUNDERCAT_CUSPARSE_HPP