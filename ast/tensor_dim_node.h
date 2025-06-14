#ifndef __UDF_AST_TENSOR_DIM_NODE_H__
#define __UDF_AST_TENSOR_DIM_NODE_H__

#include <cdk/ast/expression_node.h>

namespace udf {

  /**
   * Class for describing tensor dimention nodes.
   */
  class tensor_dim_node: public cdk::expression_node {
    cdk::expression_node *_tensor;
    cdk::expression_node *_dim;
  public:
    tensor_dim_node(int lineno, cdk::expression_node *tensor, cdk::expression_node *dim) :
        cdk::expression_node(lineno), _tensor(tensor), _dim(dim) {}
    
    cdk::expression_node *dim() {
      return _dim;
    }
    
    cdk::expression_node *tensor() {
      return _tensor;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tensor_dim_node(this, level);
    }

  };

} // udf

#endif