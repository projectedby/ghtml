#include <string>
#include <iostream>
#include <list>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <vector>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include <libxml/parser.h>

// Open a new context and module.
static std::unique_ptr<llvm::LLVMContext> TheContext = std::make_unique<llvm::LLVMContext>();
static std::unique_ptr<llvm::Module> TheModule = std::make_unique<llvm::Module>("GNU LIKE HTML COMPILER", *TheContext);

// Create a new builder for the module.
static std::unique_ptr<llvm::IRBuilder<>> Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);

class Token
{
protected:  std::string _tag;
protected:  std::string _endl;
protected:  std::list<Token *> _children;
public:     virtual llvm::Value * build(){
                return nullptr;
            }
public:     virtual llvm::Value * codegen() = 0;
public:     virtual void add(std::list<Token *> & children)
            {
                for(std::list<Token *>::iterator it = children.begin(); it != children.end(); ++it)
                {
                    _children.push_back(*it);
                }
            }
public:     const std::string & tag(void) const { return this->_tag; }
public:     virtual std::string get(void)
            {
                return "";
            }
public:     virtual void print(void)
            {
                // std::cout << _tag << _endl;
            }
public:     Token(const char * tag)
            {
                this->_tag = tag;
                _endl = ";\n";
            }
public:     virtual ~Token()
            {
                for(std::list<Token *>::iterator it = _children.begin(); it != _children.end(); ++it) {
                    Token * token = *it;
                    if(token) {
                        delete token;
                    }
                }
            }
};

class Declare : public Token
{
public:     static std::map<std::string, llvm::AllocaInst *> variables;
protected:  std::string _type;
protected:  std::string _name;
protected:  std::string _value;
public:     virtual llvm::Value * codegen()
            {
                return NULL;
            }
public:     const std::string & type(void) const { return _type; }
public:     const std::string & name(void) const { return _name; }
public:     const std::string & value(void) const { return _value; }
public:     virtual std::string get()
            {
                return _type + " " + _name + " = " + _value;
            }
public:     virtual void print(void)
            {
                // std::cout << _type << " " << _name << " = " << _value << ";" << std::endl;
            }

public:     virtual llvm::Value * build()
            {
                llvm::Value * constant = nullptr;
                llvm::AllocaInst * alloca = nullptr;
                if(_type == "int") {
                    constant = llvm::ConstantInt::get(*TheContext, llvm::APInt(32, std::stoi(_value), true));
                    alloca = Builder->CreateAlloca(llvm::Type::getInt32Ty(*TheContext), nullptr, _name.c_str());

                    variables[_name] = alloca;

                    return Builder->CreateStore(constant, alloca);
                } else {
                    std::cout << "current not support" << std::endl;
                }
                return nullptr;
            }
public:     Declare(void) : Token("declare") {}
public:     Declare(const char * type, const char * name, const char * value) : Token("declare")
            {
                _type = type;
                _name = name;
                _value = value;
            }
public:     virtual ~Declare(){}
};

std::map<std::string, llvm::AllocaInst *> Declare::variables;

class Variable : public Token
{
protected:  std::string _name;
public:     const std::string & name(void) const { return _name; }
public:     virtual llvm::Value * codegen()
            {
                return NULL;
            }
public:     virtual llvm::Value * build(){
                llvm::AllocaInst * alloca = Declare::variables[_name];
                return Builder->CreateLoad(alloca->getAllocatedType(), alloca, _name.c_str());
            }
public:     virtual std::string get()
            {
                return _name;                
            }
public:     Variable(void) : Token("variable") {}
public:     Variable(const char * name) : Token("variable")
            {
                _name = name;
            }
public:     virtual ~Variable(void)
            {
                
            }
};

class Operator : public Token
{
protected:  std::string _name;
public:     const std::string & name(void) const { return _name; }
public:     virtual llvm::Value * codegen()
            {
                return NULL;
            }
public:     virtual std::string get()
            {
                return _name;                
            }
public:     Operator(void) : Token("operator") {}
public:     Operator(const char * name) : Token("operator")
            {
                _name = name;
            }
public:     virtual ~Operator(void)
            {
                
            }
};



class Return : public Token
{
public:     virtual llvm::Value * codegen()
            {
                return NULL;
            }
public:     virtual llvm::Value * build()
            {
                llvm::Value * left = nullptr;
                llvm::Value * right = nullptr;
                Operator * oper = nullptr;

                for(std::list<Token *>::iterator it = _children.begin(); it != _children.end(); ++it) {
                    if(left == nullptr) {
                        left = (*it)->build();
                        continue;
                    }
                    if((*it)->tag() == "operator") {
                        oper = static_cast<Operator*>(*it);
                        continue;
                    }
                    if(oper->name() == "+") {
                        right = (*it)->build();
                        left = Builder->CreateAdd(left, right, "ret");
                    } else {
                        std::cout << "not supported" <<std::endl;
                    }
                }

                return Builder->CreateRet(left);
            }
public:     virtual std::string get(void)
            {
                std::string out = "return ";

                for(std::list<Token *>::iterator it = _children.begin(); it != _children.end(); ++it)
                {
                    out += (*it)->get() + " ";
                }

                return out;
            }
public:     Return(void) : Token("return") {}
public:     virtual ~Return(){}
};

class Document
{
protected:  std::list<Token *> tokens;
public:     virtual llvm::Value * build()
            {
                for(std::list<Token *>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
                    Token * token = *it;
                    token->build();
                    // std::cout << token->name() << std::endl;
                }
                std::error_code code;

                llvm::raw_fd_ostream output("hello.bc", code);

                TheModule->print(output, nullptr);

                return nullptr;
            }
public:     virtual llvm::Value * codegen()
            {
                return NULL;
            }
public:     void add(Token * token)
            {
                tokens.push_back(token);
            }
public:     void print(void)
            {
                for(std::list<Token *>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
                    Token * token = *it;
                    token->print();
                    // std::cout << token->name() << std::endl;
                }
            }
public:     Document(){}
public:     virtual ~Document()
            {
                for(std::list<Token *>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
                    Token * token = *it;
                    if(token) {
                        delete token;
                    }
                }
            }
};

static Document document;

static std::list<Token *> parse(xmlNodePtr parent, xmlDocPtr doc) {
    std::list<Token *> tokens;
    xmlNodePtr cur = parent->xmlChildrenNode;
    while(cur != NULL) {
        if(!xmlStrcmp(cur->name, (const xmlChar *) "text")) {
            cur = cur->next;
            continue;
        }

        if(!xmlStrcmp(cur->name, (const xmlChar *) "ghtml-variable")) {
            xmlChar * _name = xmlGetProp(cur, (const xmlChar *) "target");

            tokens.push_back(new Variable((const char *) _name));

            xmlFree(_name);

            cur = cur->next;

            continue;
        }
        if(!xmlStrcmp(cur->name, (const xmlChar *) "ghtml-operator")) {
            xmlChar * _type = xmlGetProp(cur, (const xmlChar *) "type");

            tokens.push_back(new Operator((const char *) _type));

            xmlFree(_type);

            cur = cur->next;
            continue;
        }
        printf("%s\n", (const char *) cur->name);

        cur = cur->next;
    }

    return tokens;
}

static void parseMain(xmlNodePtr parent, xmlDocPtr doc) {
    xmlNodePtr cur = parent->xmlChildrenNode;
    while(cur != NULL) {
        if(!xmlStrcmp(cur->name, (const xmlChar *) "text")) {
            cur = cur->next;
            continue;
        }

        if(!xmlStrcmp(cur->name, (const xmlChar *) "ghtml-declare")) {
            printf("%s\n", cur->name);

            xmlChar * __type = xmlGetProp(cur, (const xmlChar *) "type");
            xmlChar * __name = xmlGetProp(cur, (const xmlChar *) "name");
            xmlChar * __value = xmlGetProp(cur, (const xmlChar *) "value");

            Declare * __declare = new Declare((const char *) __type, (const char *) __name, (const char *) __value);

            printf("%s\n", (const char *) __type);
            printf("%s\n", (const char *) __name);
            printf("%s\n", (const char *) __value);

            xmlFree(__type);
            xmlFree(__name);
            xmlFree(__value);

            printf("%p\n", cur->xmlChildrenNode);

            document.add(__declare);

            cur = cur->next;
            continue;
        }

        if(!xmlStrcmp(cur->name, (const xmlChar *) "ghtml-return")) {
            printf("%s\n", cur->name);
            printf("%p\n", cur->xmlChildrenNode);

            Return * __return = new Return();

            if(cur->xmlChildrenNode) {
                std::list<Token *> children = parse(cur, doc);
                __return->add(children);
            }

            // std::cout << "return => " << __return->get() << std::endl;

            document.add(__return);

            cur = cur->next;
            continue;
        }

        printf("hello world\n");

        xmlBufferPtr buffer = xmlBufferCreate();
        int n = xmlNodeDump(buffer, doc, cur, 0, 1);

        printf("%s\n", buffer->content);

        xmlBufferFree(buffer);
        cur = cur->next;
    }
}



int main(int argc, char ** argv)
{
    if(argc < 2) {
        // GHTML을 실행시키기 위해서 HTML 파일이 필요합니다.
        // To run HTML, you need an HTML file.
        std::cout << "You need a html file." << std::endl << std::endl;
        std::cout << "    ghtml [filename].html" << std::endl;
        std::cout << "" << std::endl;
        return 0;
    }

    xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(argv[1]);

    if(doc == NULL) {
        fprintf(stderr,"Document not parsed successfully. \n");
        return -1;
    }

    cur = xmlDocGetRootElement(doc);

    if(cur == NULL) {
        fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return -1;
    }

    if (xmlStrcmp(cur->name, (const xmlChar *) "html")) {
        std::cout << "need to html" << std::endl;
        return -1;
    }

    cur = cur->xmlChildrenNode;

    while(cur != NULL) {
        if (!xmlStrcmp(cur->name, (const xmlChar *) "text")) {
            cur = cur->next;
            continue;
        }
        printf("%s\n", cur->name);
        if(!xmlStrcmp(cur->name, (const xmlChar *) "head")) {
            cur = cur->next;
            continue;
        }
        
        if(!xmlStrcmp(cur->name, (const xmlChar *) "body")) {
            parseMain(cur, doc);
            cur = cur->next;
            continue;
        }

        printf("hello world");

        xmlBufferPtr buffer = xmlBufferCreate();
        int n = xmlNodeDump(buffer, doc, cur, 0, 1);

        printf("%s\n", buffer->content);

        xmlBufferFree(buffer);
        cur = cur->next;
    }

    // document.print();

    llvm::FunctionType * functionType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*TheContext), std::vector<llvm::Type*>(),false);
    llvm::Function * function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "main", TheModule.get());
    llvm::BasicBlock * block = llvm::BasicBlock::Create(*TheContext, "entry", function);
    Builder->SetInsertPoint(block);

    document.build();

    return 0;
}