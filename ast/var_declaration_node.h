#ifndef __UDF_AST_VARIABLE_DECLARATION_H__
#define __UDF_AST_VARIABLE_DECLARATION_H__

#include <cdk/ast/typed_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/types/basic_type.h>
#include <string>

namespace udf {

	/**
	 * Class for describing declaration nodes.
	*/
  class var_declaration_node: public cdk::typed_node {
    int _qualifier;
    std::string _identifier;
    cdk::expression_node *_init;

  public:
    var_declaration_node(int lineno, int qualifier, std::shared_ptr<cdk::basic_type> varType, 
            const std::string &identifier, cdk::expression_node *initializer) :
        cdk::typed_node(lineno), _qualifier(qualifier), _identifier(identifier), _init(initializer) {
        cdk::typed_node::type(varType);
    }

    int qualifier() {
      return _qualifier;
    }
		
    const std::string& identifier() const {
      return _identifier;
    }
		
    cdk::expression_node* init() {
      return _init;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_var_declaration_node(this, level);
    }

  };

} // udf

#endif
