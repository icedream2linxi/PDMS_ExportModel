using Aveva.Pdms.Database;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ExportModel
{
	public class Experssion
	{
		private string exper;
		private IOperator op = null;
		public string Exper
		{
			get
			{
				return exper;
			}
		}

		public DbElement ModelElement { get; set; }

		public Experssion(string exper)
		{
			this.exper = exper.ToUpper();
		}

		private bool IsSplit(char ch)
		{
			return Char.IsWhiteSpace(ch) || ch == '[' || ch == ']';
		}

		private bool SkipSpace(CharEnumerator iter)
		{
			while(IsSplit(iter.Current))
			{
				if (!iter.MoveNext())
					return false;
			}
			return true;
		}

		private bool IsOp(char ch)
		{
			return ch == '+' || ch == '-' || ch == '*' || ch == '/';
		}

		public void Parse()
		{
			CharEnumerator experIter = Exper.GetEnumerator();
			Parse(experIter, ref op);
		}

		public double Eval(DbElement ele)
		{
			ModelElement = ele;
			return op.Eval();
		}

		enum OperatorOrder
		{
			NONE,
			NEED_OPERATOR,
			NEED_VALUE
		};

		private bool Parse(CharEnumerator experIter, IOperator prevOp, ref IOperator op)
		{
			if (!SkipSpace(experIter))
				return true;

			bool isEof = false;
			if (IsOp(experIter.Current))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, ref valueOp);
				switch (experIter.Current)
				{
					case '-':
						{
							if (!isEof)
							{
								IOperator nextValueOp = null;
								isEof = Parse(experIter, valueOp, ref nextValueOp);
								if (nextValueOp == null)
									op = new SubOp(prevOp, valueOp);
								else
									op = new SubOp(prevOp, nextValueOp);
							}
							else
								op = new SubOp(prevOp, valueOp);
							break;
						}
					case '+':
						{
							if (!isEof)
							{
								IOperator nextValueOp = null;
								isEof = Parse(experIter, valueOp, ref nextValueOp);
								if (nextValueOp == null)
									op = new AddOp(prevOp, valueOp);
								else
									op = new AddOp(prevOp, nextValueOp);
							}
							else
								op = new AddOp(prevOp, valueOp);
							break;
						}
					case '*':
						op = new MulOp(prevOp, valueOp);
						break;
					case '/':
						op = new DivOp(prevOp, valueOp);
						break;
					default:
						throw new FormatException();
				}

				return isEof;
			}
			else if (experIter.Current == '('
				|| experIter.Current == ')')
			{
				throw new FormatException();
			}
			else
			{
				isEof = ParseItem(experIter, OperatorOrder.NEED_OPERATOR, ref op);
				if (op is TwoObjOp)
					((TwoObjOp)op).LhsItem = prevOp;
			}
			return false;
		}

		private bool Parse(CharEnumerator experIter, ref IOperator op)
		{
			op = null;
			if (!SkipSpace(experIter))
				return true;

			bool isEof = false;
			if (IsOp(experIter.Current))
			{
				if (experIter.Current == '-')
				{
					IOperator valueOp = null;
					isEof = Parse(experIter, ref valueOp);
					if (valueOp == null)
						throw new FormatException();
					op = new NegOp(valueOp);
				}
				else if (experIter.Current == '+')
				{
					isEof = Parse(experIter, ref op);
				}
				else
					throw new FormatException();
			}
			else if (experIter.Current == '(')
			{
				if (!SkipSpace(experIter))
					throw new FormatException();
				isEof = Parse(experIter, ref op);
			}
			else if (experIter.Current == ')')
			{
				return false;
			}
			else
			{
				isEof = ParseItem(experIter, OperatorOrder.NEED_VALUE, ref op);
			}

			while (!isEof)
			{
				IOperator nextValueOp = null;
				isEof = Parse(experIter, op, ref nextValueOp);
				if (nextValueOp != null)
					op = nextValueOp;
			}

			return isEof;
		}

		private bool ParseItem(CharEnumerator experIter, OperatorOrder opOrder, ref IOperator op)
		{
			StringBuilder sb = new StringBuilder();
			bool isEof = false;
			while (!IsSplit(experIter.Current) && !IsOp(experIter.Current) && experIter.Current != '(' && experIter.Current != ')')
			{
				sb.Append(experIter.Current);
				if (!experIter.MoveNext())
				{
					isEof = true;
					break;
				}
			}

			string item = sb.ToString();
			if (Char.IsDigit(item[0]) || item[0] == '-' || item[0] == '+' || item[0] == '.')
			{
				op = new ValOp(Double.Parse(item));
				return isEof;
			}

			if (item.Equals("DDHEIGHT"))
			{
				op = new HeightOp(this);
				return isEof;
			}
			else if (item.Equals("DDANGLE"))
			{
				op = new AngleOp(this);
				return isEof;
			}
			else if (item.Equals("DDRADIUS"))
			{
				op = new RadiusOp(this);
				return isEof;
			}

			if (isEof)
				throw new FormatException();

			if (item.Equals("TIMES"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, ref valueOp);
				op = new MulOp(null, valueOp);
			}
			else if (item.Equals("DIFFERENCE"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, ref valueOp);
				IOperator nextValueOp = null;
				isEof = Parse(experIter, valueOp, ref nextValueOp);
				if (nextValueOp == null)
					op = new SubOp(null, valueOp);
				else
					op = new SubOp(null, nextValueOp);
			}
			else if (item.Equals("SUM"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, ref valueOp);
				IOperator nextValueOp = null;
				isEof = Parse(experIter, valueOp, ref nextValueOp);
				if (nextValueOp == null)
					op = new AddOp(null, valueOp);
				else
					op = new AddOp(null, nextValueOp);
			}
			else if (item.Equals("PARAM"))
			{
				ParamOp paramOp = new ParamOp(this);
				IOperator valueOp = null;
				isEof = Parse(experIter, ref valueOp);
				paramOp.Item = valueOp;
				return isEof;
			}
			else if (item.Equals("IPARAM"))
			{
				IParamOp paramOp = new IParamOp(this);
				IOperator valueOp = null;
				isEof = Parse(experIter, ref valueOp);
				paramOp.Item = valueOp;
				return isEof;
			}
			else if (item.Equals("DESIGN"))
			{
				while (!IsSplit(experIter.Current) && !IsOp(experIter.Current) && experIter.Current != '(' && experIter.Current != ')')
				{
					sb.Append(experIter.Current);
					if (!experIter.MoveNext())
					{
						isEof = true;
						break;
					}
				}

				if (isEof)
					throw new FormatException();

				if (item.Equals("PARAM"))
				{
					DParamOp paramOp = new DParamOp(this);
					IOperator valueOp = null;
					isEof = Parse(experIter, ref valueOp);
					paramOp.Item = valueOp;
					return isEof;
				}
				else
					throw new FormatException();
			}

			return false;
		}
	}

	interface IOperator
	{
		public double Eval();
	}

	abstract class OneObjOp : IOperator
	{
		public OneObjOp()
		{

		}

		public OneObjOp(IOperator item)
		{
			Item = item;
		}

		public IOperator Item { get; set; }
		public double Eval()
		{
			throw new NotImplementedException();
		}
	}

	abstract class TwoObjOp : IOperator
	{
		public TwoObjOp()
		{

		}

		public TwoObjOp(IOperator lhs, IOperator rhs)
		{
			LhsItem = lhs;
			RhsItem = rhs;
		}

		public IOperator LhsItem { get; set; }
		public IOperator RhsItem { get; set; }

		public double Eval()
		{
			throw new NotImplementedException();
		}
	}

	class AddOp : TwoObjOp
	{
		public AddOp()
		{

		}

		public AddOp(IOperator lhs, IOperator rhs) : base(lhs, rhs)
		{

		}

		public double Eval()
		{
			return LhsItem.Eval() + RhsItem.Eval();
		}
	}

	class SubOp : TwoObjOp
	{
		public SubOp()
		{

		}

		public SubOp(IOperator lhs, IOperator rhs) : base(lhs, rhs)
		{

		}

		public double Eval()
		{
			return LhsItem.Eval() / RhsItem.Eval();
		}
	}

	class MulOp : TwoObjOp
	{
		public MulOp()
		{

		}

		public MulOp(IOperator lhs, IOperator rhs) : base(lhs, rhs)
		{

		}

		public double Eval()
		{
			return LhsItem.Eval() * RhsItem.Eval();
		}
	}

	class DivOp : TwoObjOp
	{
		public DivOp()
		{

		}

		public DivOp(IOperator lhs, IOperator rhs) : base(lhs, rhs)
		{

		}

		public double Eval()
		{
			return LhsItem.Eval() / RhsItem.Eval();
		}
	}

	class NegOp : OneObjOp
	{
		public NegOp()
		{

		}

		public NegOp(IOperator item) : base(item)
		{

		}

		public double Eval()
		{
			return -Item.Eval();
		}
	}

	class ValOp : IOperator
	{
		public double Val { get; set; }

		public ValOp(double val)
		{
			Val = val;
		}

		public double Eval()
		{
			return Val;
		}
	}

	abstract class EleOp : OneObjOp
	{
		public Experssion Exper { get; set; }
		public EleOp(Experssion exper)
		{
			Exper = exper;
		}
	}

	class ParamOp : EleOp
	{
		public ParamOp(Experssion exper) : base(exper)
		{
		}

		public double Eval()
		{
			DbElement spref = Exper.ModelElement.GetElement(DbAttributeInstance.SPRE);
			DbElement cate = spref.GetElement(DbAttributeInstance.CATR);
			return cate.GetDoubleArray(DbAttributeInstance.PARA)[(int)Item.Eval()];
		}
	}

	class DParamOp : EleOp
	{
		public DParamOp(Experssion exper)
			: base(exper)
		{
		}

		public double Eval()
		{
			return Exper.ModelElement.GetDoubleArray(DbAttributeInstance.PARA)[(int)Item.Eval()];
		}
	}

	class IParamOp : EleOp
	{
		public IParamOp(Experssion exper)
			: base(exper)
		{
		}

		public double Eval()
		{
			return Exper.ModelElement.GetElement(DbAttributeInstance.ISPE).GetDoubleArray(DbAttributeInstance.IPAR)[(int)Item.Eval()];
		}
	}

	class HeightOp : EleOp
	{
		public HeightOp(Experssion exper) : base(exper)
		{

		}

		public double Eval()
		{
			return Exper.ModelElement.GetDouble(DbAttributeInstance.HEIG);
		}
	}

	class AngleOp : EleOp
	{
		public AngleOp(Experssion exper)
			: base(exper)
		{

		}

		public double Eval()
		{
			return Exper.ModelElement.GetDouble(DbAttributeInstance.ANGL);
		}
	}

	class RadiusOp : EleOp
	{
		public RadiusOp(Experssion exper)
			: base(exper)
		{

		}

		public double Eval()
		{
			return Exper.ModelElement.GetDouble(DbAttributeInstance.RADI);
		}
	}
}
