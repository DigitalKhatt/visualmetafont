#ifndef PATHPOINTEXP_HPP
#define PATHPOINTEXP_HPP

#include <memory>
#include "qsharedpointer.h"
#include "qvariant.h"
#include "qpoint.h"
#include <stdexcept>

#include "qmath.h"

enum class  MFExprOperator {
	PLUS,
	MINUS,
	TIMES,
	OVER
};


class MFExpr {
public:
	virtual ~MFExpr() = default;

	virtual QString toString() = 0;

	virtual QVariant constantValue(int i) {
		return QVariant();
	}

	virtual QVariant::Type type() { return QVariant::Invalid; }

	virtual bool isConstant(int i) = 0;

	virtual bool containsConstant() = 0;

	virtual void setConstantValue(int i, QVariant value) {}


	virtual QString paramName(int i) {
		return QString();
	}

	virtual bool containsParam() {
		return false;
	}

	virtual std::unique_ptr<MFExpr> clone() = 0;

	virtual bool isLiteral() { return false; }

	virtual bool isVar() { return false; }
};

class PathPointExp : public MFExpr {
public:
	QVariant::Type type() override { return QVariant::PointF; }
};

class PathNumericExp : public MFExpr {
public:
	virtual QVariant::Type type() { return QVariant::Double; }
};

class LitPathNumericExp :public PathNumericExp {
private:
	double val;

public:
	LitPathNumericExp(double v) :val(v) { }

	QString toString() override {
		return QString("%1").arg(val);
	}

	QVariant constantValue(int i) override {
		return val;
	}

	void setConstantValue(int i, QVariant value) override {
		val = value.toDouble();
	}

	QVariant::Type type() override {
		return QVariant::Double;
	}



	bool isConstant(int i) override {
		return true;
	}

	bool containsConstant() override {
		return true;
	}

	std::unique_ptr<MFExpr> clone() override {
		return std::make_unique<LitPathNumericExp>(val);
	}

	bool isLiteral() override { return true; }
};

class VarMFExpr :public MFExpr {
private:
	QString val;
	bool _isParam;
public:

	VarMFExpr(QString v, bool isParam) :val{ v }, _isParam{ isParam }{ }

	// Returns a reference to the stored string value.
	QString toString() override {
		return val;
	}

	bool isConstant(int i) override {
		return false;
	}

	bool containsConstant() override {
		return false;
	}

	QVariant::Type type() override {
		return QVariant::String;
	}

	std::unique_ptr<MFExpr> clone() override {
		return std::make_unique<VarMFExpr>(val, _isParam);
	}

	bool containsParam() override {
		return _isParam;
	}

	QString paramName(int i) override {
		if (_isParam) {
			return toString();
		}
		else {
			return QString();
		}
	}

	virtual bool isVar() { return true; }
};




class DirPathPointExp :public PathPointExp {
private:
	std::unique_ptr<MFExpr> val;
public:

	DirPathPointExp(MFExpr* v) :val{ v } { }

	MFExpr* getVal() {
		return val.get();
	}

	void setVal(MFExpr* expr) {
		val = std::unique_ptr<MFExpr>(expr);
	}

	// Returns a reference to the stored string value.
	QString toString() override {
		return QString("dir %1").arg(val->toString());
	}

	bool isConstant(int i) override {
		return val && val->isConstant(i);
	}

	bool containsConstant() override {
		return val && val->containsConstant();
	}

	QVariant::Type type() override {
		return QVariant::PointF;
	}

	QVariant constantValue(int i) override {
		QVariant ret;
		//if (val && val->type() == QVariant::Double) {
		auto angleDegree = val->constantValue(i).toDouble();
		auto angleRadian = qDegreesToRadians(angleDegree);
		ret = QPointF{ qCos(angleRadian),qSin(angleRadian) };
		//}
		return ret;
	}

	void setConstantValue(int i, QVariant value) override {
		//if (val && val->type() == QVariant::Double && value.type() == QVariant::PointF) {
		auto point = value.toPointF();
		auto angleRadian = qAtan2(point.y(), point.x());
		auto angleDegree = qRadiansToDegrees(angleRadian);
		val->setConstantValue(i, angleDegree);
		//}
	}

	std::unique_ptr<MFExpr> clone() override {
		if (val) {
			auto newVal = val->clone();
			return std::make_unique<DirPathPointExp>(newVal.release());
		}
		else {
			return std::make_unique<DirPathPointExp>(nullptr);
		}

	}

	bool containsParam() override {
		return val && val->containsParam();
	}

	QString paramName(int i) override {
		if (val) {
			return val->paramName(i);
		}
		else {
			return QString();
		}
	}
};

class PairPathPointExp :public PathPointExp {
private:
	std::unique_ptr<MFExpr> left;
	std::unique_ptr<MFExpr> right;

public:
	PairPathPointExp(QPointF v) {
		left = std::make_unique<LitPathNumericExp>(v.x());
		right = std::make_unique<LitPathNumericExp>(v.y());
	}

	PairPathPointExp(MFExpr* left, MFExpr* right) {
		this->left = std::unique_ptr<MFExpr>(left);
		this->right = std::unique_ptr<MFExpr>(right);
	}

	QString toString() override {
		return QString("(%1,%2)").arg(left->toString()).arg(right->toString());
	}

	QVariant constantValue(int i) override {
		return QPointF{ left->constantValue(i).toDouble(),right->constantValue(i).toDouble() };
	}

	void setConstantValue(int i, QVariant value) override {
		auto point = value.toPointF();
		left->setConstantValue(i, point.x());
		right->setConstantValue(i, point.y());
	}

	QVariant::Type type() override {
		return QVariant::PointF;
	}

	bool isConstant(int i) override {
		return left->isConstant(i) && right->isConstant(i);
	}

	bool containsConstant() override {
		return left->isConstant(0) && right->isConstant(0);
	}

	std::unique_ptr<MFExpr> clone() override {
		auto newleft = left->clone();
		auto newright = right->clone();
		return std::make_unique<PairPathPointExp>(newleft.release(), newright.release());
	}

	bool isLiteral() override { return left->isLiteral() && right->isLiteral(); }
};


class LitPointPathPointExp :public PathPointExp {
private:
	QPointF val;

public:
	LitPointPathPointExp(QPointF v) :val(v) { }

	QString toString() override {
		return QString("(%1,%2)").arg(val.x()).arg(val.y());
	}

	QVariant constantValue(int i) override {
		return val;
	}

	void setConstantValue(int i, QVariant value) override {
		val = value.toPointF();
	}

	QVariant::Type type() override {
		return QVariant::PointF;
	}



	bool isConstant(int i) override {
		return true;
	}

	bool containsConstant() override {
		return true;
	}

	std::unique_ptr<MFExpr> clone() {
		return std::make_unique<LitPointPathPointExp>(val);
	}

	bool isLiteral() override { return true; }
};

class BinOpMFExp :public PathPointExp {

protected:
	std::unique_ptr<MFExpr> args[2];
	MFExprOperator op;

public:
	BinOpMFExp(MFExpr* l, MFExprOperator o, MFExpr* r) : op{ o }
	{
		args[0] = std::unique_ptr<MFExpr>(l);
		args[1] = std::unique_ptr<MFExpr>(r);

	}

	// Returns a reference to the stored string value.
	QString toString() override {

		QString opString;

		switch (op) {
		case  MFExprOperator::PLUS:
			opString = "+";
			break;
		case  MFExprOperator::MINUS:
			opString = "-";
			break;
		case  MFExprOperator::TIMES:
			opString = "*";
			break;
		case  MFExprOperator::OVER:
			opString = "/";
			break;
		default:
			opString = "+";
		}
		return args[0]->toString() + " " + opString + " " + args[1]->toString();
	}

	QVariant constantValue(int pos) override {
		if (pos < 2) {
			return args[pos]->constantValue(0);
		}
	}

	bool isConstant(int pos) override {
		if (pos < 2) {
			return args[pos]->isConstant(0);
		}
	}

	virtual void setConstantValue(int pos, QVariant value) override {
		if (pos < 2 && args[pos]->isConstant(0)) {
			args[pos]->setConstantValue(0, value);
		}
	}

	bool containsConstant() override {
		return args[0]->containsConstant() || args[1]->containsConstant();
	}

	std::unique_ptr<MFExpr> clone() {
		auto exp1 = args[0]->clone();
		auto exp2 = args[1]->clone();
		return std::make_unique<BinOpMFExp>(exp1.release(), op, exp2.release());
	}

	bool containsParam() override {
		return args[0]->containsParam() || args[1]->containsParam();
	}

	QString paramName(int i) override {
		if (i < 2) {
			return args[i]->paramName(0);
		}
	}


};

class FunctionMFExp :public PathPointExp {

protected:
	std::unique_ptr<MFExpr> args[2];
	QString functionName;
	int nbArgs;

public:
	FunctionMFExp(QString functionName, MFExpr* l, MFExpr* r) : functionName{ functionName }
	{
		args[0] = std::unique_ptr<MFExpr>(l);
		args[1] = std::unique_ptr<MFExpr>(r);

		nbArgs = 2;


	}

	MFExpr* getFirst() {
		return args[0].get();
	}

	MFExpr* getSecond() {
		return args[1].get();
	}

	// Returns a reference to the stored string value.
	QString toString() override {
		return QString("%1(%2,%3)").arg(functionName).arg(args[0]->toString()).arg(args[1]->toString());
	}

	QVariant constantValue(int pos) override {
		if (pos < nbArgs) {
			return args[pos]->constantValue(0);
		}
	}

	bool isConstant(int pos) override {
		if (pos < nbArgs) {
			return args[pos]->isConstant(0);
		}
	}

	virtual void setConstantValue(int pos, QVariant value) override {
		if (pos < nbArgs && args[pos]->isConstant(0)) {
			args[pos]->setConstantValue(0, value);
		}
	}

	bool containsConstant() override {
		for (int i = 0; i < nbArgs; i++) {
			if (args[i]->containsConstant()) {
				return true;
			}
		}
		return false;
	}



	bool containsParam() override {
		for (int i = 0; i < nbArgs; i++) {
			if (args[i]->containsParam()) {
				return true;
			}
		}
		return false;
	}

	QString paramName(int i) override {
		if (i < 2) {
			return args[i]->paramName(0);
		}
	}

	std::unique_ptr<MFExpr> clone() override {
		auto exp1 = args[0]->clone();
		auto exp2 = args[1]->clone();
		auto ret = std::make_unique<FunctionMFExp>(functionName, exp1.release(), exp2.release());
		return ret;
	}

};


class ParenthesesMFExp :public PathNumericExp {
protected:
	std::unique_ptr<MFExpr> child;

public:
	ParenthesesMFExp(MFExpr* child)
	{
		this->child = std::unique_ptr<MFExpr>(child);
	}

	QString toString() override {
		return "(" + child->toString() + ")";
	}

	QVariant constantValue(int pos) override {
		return child->constantValue(pos);
	}

	bool isConstant(int pos) override {
		return child->isConstant(pos);
	}

	virtual void setConstantValue(int pos, QVariant value) override {
		child->setConstantValue(pos, value);
	}

	bool containsConstant() override {
		return child->containsConstant();
	}

	bool containsParam() override {
		return child->containsParam();
	}

	QString paramName(int i) override {
		return child->paramName(i);
	}

	std::unique_ptr<MFExpr> clone() override {
		auto exp1 = child->clone();
		return std::make_unique<ParenthesesMFExp>(exp1.release());
	}
};

class ScalarMultiMFExp :public PathNumericExp {
protected:
	std::unique_ptr<MFExpr> child;
	MFExprOperator op;


public:
	ScalarMultiMFExp(MFExpr* child, MFExprOperator op) : op{ op }
	{
		this->child = std::unique_ptr<MFExpr>(child);
	}

	QString toString() override {

		QString opString;
		switch (op) {
		case  MFExprOperator::PLUS:
			opString = "+";
			break;
		case  MFExprOperator::MINUS:
			opString = "-";
			break;
		default:
			opString = "+";
		}
		return opString + child->toString();
	}

	QVariant constantValue(int pos) override {
		return child->constantValue(pos);
	}

	bool isConstant(int pos) override {
		return child->isConstant(pos);
	}

	virtual void setConstantValue(int pos, QVariant value) override {
		child->setConstantValue(pos, value);
	}

	bool containsConstant() override {
		return child->containsConstant();
	}

	bool containsParam() override {
		return child->containsParam();
	}

	QString paramName(int i) override {
		return child->paramName(i);
	}

	std::unique_ptr<MFExpr> clone() override {
		auto exp1 = child->clone();
		return std::make_unique<ScalarMultiMFExp>(exp1.release(), op);
	}
};




#endif // PATHPOINTEXP_HPP
