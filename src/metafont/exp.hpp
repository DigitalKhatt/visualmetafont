#ifndef EXP_HPP
#define EXP_HPP

#include <memory>
#include "qsharedpointer.h"
#include "qvariant.h"
#include "qpoint.h"

enum class  Oper {
  ADD
};

class Exp {
 public:
  /* This is the method that must be overridden by all subclasses.
   * It should perform the computation specified by this node, and
   * return the resulting value that gets computed. */
  virtual QString toString() {return "";}
  virtual QVariant constantValue(){
    QVariant value;
    return value;
  }
  virtual void setConstantValue(QVariant value){}
  virtual QVariant value(){
    QVariant value;
    return value;
  }
  virtual void setValue(QVariant value){}
  virtual QVariant::Type type(){return QVariant::Invalid;}
};

class Id :public Exp {
  private:
    QString val;
  public:

    Id(QString v) :val(v) { }

    // Returns a reference to the stored string value.
    QString toString() override {
      return val;
    }
};

class LitNumber :public Exp {
  private:
    double val;

  public:
    LitNumber(double v) :val(v) { }

    QString toString() override {
      return QString("%1").arg(val);
    }

    QVariant constantValue() override {
      return QVariant(val);
    }
    virtual void setConstantValue(QVariant value) override{
      if(value.type() == QVariant::Double){
        val = value.toDouble();
      }
    }
    QVariant value() override {
       return constantValue();
    }
    void setValue(QVariant value) override {
      setConstantValue(value);
    }

    QVariant::Type type() override{
      return QVariant::Double;
    }
};

class DirNumber :public Exp {
  private:
    double val;

  public:
    DirNumber(double v) :val(v) { }

    QString toString() override {
      return QString("dir %1").arg(val);
    }

    QVariant constantValue() override {
      return QVariant(val);
    }
    virtual void setConstantValue(QVariant value) override{
      if(value.type() == QVariant::Double){
        val = value.toDouble();
      }
    }
    QVariant value() override {
       return constantValue();
    }
    void setValue(QVariant value) override {
      setConstantValue(value);
    }

    QVariant::Type type() override{
      return QVariant::Double;
    }
};

class LitPoint :public Exp {
  private:
    QPointF val;

  public:
    LitPoint(QPointF v) :val(v) { }

    QString toString() override {
      return QString("(%1,%2)").arg(val.x()).arg(val.y());
    }

    QVariant constantValue() override {
      return QVariant(val);
    }
    virtual void setConstantValue(QVariant value) override{
      if(value.type() == QVariant::PointF){
        val = value.toPointF();
      }
    }
    QVariant value() override {
       return constantValue();
    }
    void setValue(QVariant value) override {
      setConstantValue(value);
    }

    QVariant::Type type() override{
      return QVariant::PointF;
    }
};

class BinOp :public Exp {
  private :
    QVariant val;
    Exp* constExp;
  protected:
    std::unique_ptr<Exp> args[2];
    Oper op;

  public:
    BinOp(Exp* l, Oper o, Exp* r) :val{},constExp{nullptr},op{o}
    {
      if(l->constantValue().isValid()){
        constExp = l;
      }else{
        constExp = r;
      }
      args[0] = std::unique_ptr<Exp>(l);
      args[1] = std::unique_ptr<Exp>(r);
    }

    // Returns a reference to the stored string value.
    QString toString() override {

      QString opString;

      switch (op) {
        case  Oper::ADD :
          opString= "+";
          break;
        default:
          opString= "+";
      }
      return args[0]->toString() + " " + opString + " " + args[1]->toString();
    }

    QVariant value() override {
       return val;
    }
    void setValue(QVariant value) override {
      val = value;
    }

    QVariant constantValue() override {
      return constExp->constantValue();
    }
    void setConstantValue(QVariant value) override{
      constExp->setValue(value);
    }

    QVariant::Type type() override{
      return constExp->type();
    }
};



#endif // EXP_HPP
