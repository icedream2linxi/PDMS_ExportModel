﻿using Aveva.Pdms.Database;
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
		enum BracketType
		{
			GENERAL,
			FUNCTION
		}
		private Stack<BracketType> bracketStack = new Stack<BracketType>();
		private bool isFunBracketEnd = false;

		private int bracketLevel = 0;

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
			Parse();
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
			try
			{
				bracketLevel = 0;
				CharEnumerator experIter = Exper.GetEnumerator();
				if (experIter.MoveNext())
					Parse(experIter, OperatorOrder.NONE, ref op);
				else
					op = new ValOp(0.0);
			}
			catch (System.Exception ex)
			{
				System.Console.WriteLine("Exper = " + Exper);
				throw;
			}
		}

		public double Eval(DbElement ele)
		{
			ModelElement = ele;
			return op.Eval();
		}

		private bool Parse(CharEnumerator experIter, IOperator prevOp, out IOperator op)
		{
			if (!SkipSpace(experIter))
			{
				op = null;
				return true;
			}

			bool isEof = false;
			char curCh = experIter.Current;
			switch (curCh)
			{
				case '-':
					isEof = ParseSub(experIter, prevOp, out op);
					break;
				case '+':
					break;
				default:
					op = null;
					break;
			}
			op = null;
			return isEof;
		}

		private bool ParseSub(CharEnumerator experIter, IOperator prevOp, out IOperator op)
		{
			IOperator nextOp;
			bool isEof = Parse(experIter, null, out nextOp);
			if (nextOp == null)
				throw new FormatException();
			if (prevOp == null)
			{
				NegOp negOp = new NegOp();
				negOp.Item = nextOp;
				op = negOp;
			}
			else
			{
				SubOp subOp = new SubOp();
				subOp.LhsItem = prevOp;
				if (!isEof)
				{
					IOperator tmpOp;
					isEof = Parse(experIter, nextOp, out tmpOp);
					if (tmpOp != null)
						nextOp = tmpOp;
				}
				subOp.RhsItem = nextOp;
				op = subOp;
			}
			return isEof;
		}

		private bool ParseAdd(CharEnumerator experIter, IOperator prevOp, out IOperator op)
		{
			IOperator nextOp;
			bool isEof = Parse(experIter, null, out nextOp);
			if (nextOp == null)
				throw new FormatException();
			if (prevOp == null)
			{
				op = nextOp;
			}
			else
			{
				AddOp addOp = new AddOp();
				addOp.LhsItem = prevOp;
				if (!isEof)
				{
					IOperator tmpOp;
					isEof = Parse(experIter, nextOp, out tmpOp);
					if (tmpOp != null)
						nextOp = tmpOp;
				}
				addOp.RhsItem = nextOp;
				op = addOp;
			}
			return isEof;
		}

		private bool ParseMul(CharEnumerator experIter, IOperator prevOp, out IOperator op)
		{
			if (prevOp == null)
				throw new FormatException();
			IOperator nextOp;
			bool isEof = Parse(experIter, null, out nextOp);
			MulOp mulOp = new MulOp();
			mulOp.LhsItem = prevOp;
			mulOp.RhsItem = nextOp;

			if (isEof)
			{
				op = mulOp;
				return isEof;
			}

			isEof = Parse(experIter, mulOp, out nextOp);
			if (nextOp != null)
				op = nextOp;
			else
				op = mulOp;
			return isEof;
		}

		private bool ParseDiv(CharEnumerator experIter, IOperator prevOp, out IOperator op)
		{
			if (prevOp == null)
				throw new FormatException();
			IOperator nextOp;
			bool isEof = Parse(experIter, null, out nextOp);
			DivOp divOp = new DivOp();
			divOp.LhsItem = prevOp;
			divOp.RhsItem = nextOp;

			if (isEof)
			{
				op = divOp;
				return isEof;
			}

			isEof = Parse(experIter, divOp, out nextOp);
			if (nextOp != null)
				op = nextOp;
			else
				op = divOp;
			return isEof;
		}

		private bool ParseStartBracket(CharEnumerator experIter, IOperator prevOp, out IOperator op)
		{
			if (prevOp != null)
				throw new FormatException();
			int currLevel = bracketLevel++;
			bool isEof = Parse(experIter, null, out op);
			if (isEof || op == null)
				throw new FormatException();
			IOperator nextOp;
			isEof = Parse(experIter, op, out nextOp);
			if (nextOp != null)
				op = nextOp;
			if (bracketLevel != currLevel)
				throw new FormatException();
			return isEof;
		}

		private bool ParseEndBracket(CharEnumerator experIter, IOperator prevOp, out IOperator op)
		{
			if (prevOp == null)
				throw new FormatException();
			--bracketLevel;
			op = null;
			return !experIter.MoveNext();
		}

		private bool ParseComma(CharEnumerator experIter, IOperator prevOp, out IOperator op)
		{
			if (prevOp == null)
				throw new FormatException();
			op = null;
			return !experIter.MoveNext();
		}

		private bool ParseValue(CharEnumerator experIter, IOperator prevOp, out IOperator op)
		{
			bool isEof = false;
			StringBuilder sb = new StringBuilder();
			if (!isEof && (Char.IsDigit(experIter.Current) || experIter.Current == '.'))
			{
				sb.Append(experIter.Current);
				isEof = !experIter.MoveNext();
			}

			op = new ValOp(Double.Parse(sb.ToString()));
			return isEof;
		}

		private bool ParseFunc(CharEnumerator experIter, IOperator prevOp, out IOperator op)
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
				isEof = ParseMul(experIter, prevOp, out op);
			}
			else if (item.Equals("DIFFERENCE"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, null, out valueOp);
				if (isEof)
					throw new FormatException();
				IOperator nextValueOp = null;
				isEof = Parse(experIter, null, out nextValueOp);
				op = new SubOp(valueOp, nextValueOp);
			}
			else if (item.Equals("SUM"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, null, out valueOp);
				if (isEof)
					throw new FormatException();
				IOperator nextValueOp = null;
				isEof = Parse(experIter, null, out nextValueOp);
				op = new AddOp(valueOp, nextValueOp);
			}
			else if (item.Equals("DIV"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, null, out valueOp);
				if (isEof)
					throw new FormatException();
				IOperator nextValueOp = null;
				isEof = Parse(experIter, null, out nextValueOp);
				op = new DivOp(valueOp, nextValueOp);
			}
			else if (item.Equals("TANF"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, null, out valueOp);
				if (isEof)
					throw new FormatException();
				IOperator nextValueOp = null;
				isEof = Parse(experIter, null, out nextValueOp);
				op = new TanfOp(valueOp, nextValueOp);
			}
			else if (item.Equals("MIN"))
			{
				isEof = !SkipSpace(experIter);
				if (isEof || experIter.Current != '(')
					throw new FormatException();
				int currBracket = bracketLevel++;
				IOperator valueOp = null;
				isEof = Parse(experIter, null, out valueOp);
				if (isEof)
					throw new FormatException();
				IOperator nextValueOp = null;
				isEof = Parse(experIter, null, out nextValueOp);
				if (currBracket != bracketLevel)
					throw new FormatException();
				op = new MinOp(valueOp, nextValueOp);
			}
			else if (item.Equals("MAX"))
			{
				isEof = !SkipSpace(experIter);
				if (isEof || experIter.Current != '(')
					throw new FormatException();
				int currBracket = bracketLevel++;
				IOperator valueOp = null;
				isEof = Parse(experIter, null, out valueOp);
				if (isEof)
					throw new FormatException();
				IOperator nextValueOp = null;
				isEof = Parse(experIter, null, out nextValueOp);
				if (currBracket != bracketLevel)
					throw new FormatException();
				op = new MaxOp(valueOp, nextValueOp);
			}
			else if (item.Equals("COS"))
			{
				isEof = !SkipSpace(experIter);
				if (isEof || experIter.Current != '(')
					throw new FormatException();
				int currBracket = bracketLevel++;
				IOperator valueOp = null;
				isEof = Parse(experIter, null, out valueOp);
				if (isEof || currBracket != bracketLevel)
					throw new FormatException();
				CosOp cosOp = new CosOp();
				cosOp.Item = valueOp;
				op = cosOp;
				return isEof;
			}
			else if (item.Equals("SIN"))
			{
				isEof = !SkipSpace(experIter);
				if (isEof || experIter.Current != '(')
					throw new FormatException();
				int currBracket = bracketLevel++;
				IOperator valueOp = null;
				isEof = Parse(experIter, null, out valueOp);
				if (isEof || currBracket != bracketLevel)
					throw new FormatException();
				SinOp sinOp = new SinOp();
				sinOp.Item = valueOp;
				op = sinOp;
				return isEof;
			}
			else if (item.Equals("SQRT"))
			{
				isEof = !SkipSpace(experIter);
				if (isEof || experIter.Current != '(')
					throw new FormatException();
				int currBracket = bracketLevel++;
				IOperator valueOp = null;
				isEof = Parse(experIter, null, out valueOp);
				if (isEof || currBracket != bracketLevel)
					throw new FormatException();
				SqrtOp sqrtOp = new SqrtOp();
				sqrtOp.Item = valueOp;
				op = sqrtOp;
				return isEof;
			}
			else if (item.Equals("INT"))
			{
				isEof = !SkipSpace(experIter);
				if (isEof || experIter.Current != '(')
					throw new FormatException();
				int currBracket = bracketLevel++;
				IOperator valueOp = null;
				isEof = Parse(experIter, null, out valueOp);
				if (isEof || currBracket != bracketLevel)
					throw new FormatException();
				IntOp intOp = new IntOp();
				intOp.Item = valueOp;
				op = intOp;
				return isEof;
			}
			else if (item.Equals("PARAM"))
			{
				ParamOp paramOp = new ParamOp(this);
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
				paramOp.Item = valueOp;
				op = paramOp;
				return isEof;
			}
			else if (item.Equals("IPARAM"))
			{
				IParamOp paramOp = new IParamOp(this);
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
				paramOp.Item = valueOp;
				op = paramOp;
				return isEof;
			}
			else if (item.Equals("DESIGN"))
			{
				isEof = !SkipSpace(experIter);
				if (isEof)
					throw new FormatException();

				sb = new StringBuilder();
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

				item = sb.ToString();
				if (item.Equals("PARAM"))
				{
					DParamOp paramOp = new DParamOp(this);
					IOperator valueOp = null;
					isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
					paramOp.Item = valueOp;
					op = paramOp;
					return isEof;
				}
				else
					throw new FormatException();
			}
			else if (item.Equals("TWICE"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
				op = new TwiceOp(valueOp);
			}
			else if (item.Equals("ATTRIB"))
			{
				isEof = !SkipSpace(experIter);
				if (isEof)
					throw new FormatException();

				sb = new StringBuilder();
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

				item = sb.ToString();

				if (item.Equals("RPRO"))
				{
					isEof = !SkipSpace(experIter);
					if (isEof)
						throw new FormatException();

					sb = new StringBuilder();
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

					item = sb.ToString();

					DbExperOp experOp = new DbExperOp(this);
					experOp.DbExper = DbExpression.Parse("ATTRIB RPRO " + item);
					op = experOp;
				}
				else
				{
					DbAttribute attr = DbAttribute.GetDbAttribute(item);
					if (attr == null)
						throw new FormatException();

					while (!isEof && Char.IsWhiteSpace(experIter.Current))
					{
						isEof = !experIter.MoveNext();
					}

					if (isEof || experIter.Current == '[')
					{
						AttribArrayOp paramOp = new AttribArrayOp(this);
						paramOp.Attr = attr;
						IOperator valueOp = null;
						isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
						paramOp.Item = valueOp;
						op = paramOp;
					}
					else
					{
						AttribOp paramOp = new AttribOp(this);
						paramOp.Attr = attr;
						op = paramOp;
					}
				}
			}
			else if (item.Equals("PL") || item.Equals("PA"))
			{
				isEof = !SkipSpace(experIter);
				if (isEof)
					throw new FormatException();

				sb = new StringBuilder();
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

				DbAttribute attr = item.Equals("PL") ? DbAttributeInstance.LOD : DbAttributeInstance.AOD;
				item = sb.ToString();
				if (!item.Equals("OD"))
					throw new FormatException();

				AttribOp paramOp = new AttribOp(this);
				paramOp.Attr = attr;
				op = paramOp;
			}
			else if (item.Equals("PLOD"))
			{
				AttribOp paramOp = new AttribOp(this);
				paramOp.Attr = DbAttributeInstance.LOD;
				op = paramOp;
			}
			else if (item.Equals("PAOD"))
			{
				AttribOp paramOp = new AttribOp(this);
				paramOp.Attr = DbAttributeInstance.AOD;
				op = paramOp;
			}
			else
				throw new FormatException();

			return isEof;
		}

		enum OperatorOrder
		{
			NONE,
			NEED_OPERATOR,
			NEED_VALUE,
			NEED_PARAM,
			NEED_BRACKET
		};

		private bool Parse(CharEnumerator experIter, IOperator prevOp, ref IOperator op, ref bool isEnd)
		{
			if (!SkipSpace(experIter))
				return true;

			char curCh = experIter.Current;
			bool isEof = false;
			if (IsOp(curCh))
			{
				isEof = !experIter.MoveNext();

				IOperator valueOp = null;
				if (isEof)
					throw new FormatException();
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
				switch (curCh)
				{
					case '-':
						{
							if (!isEof)
							{
								IOperator nextValueOp = null;
								//isEof = !experIter.MoveNext();
								if (!isEof)
									isEof = Parse(experIter, valueOp, ref nextValueOp, ref isEnd);
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
								//isEof = !experIter.MoveNext();
								if (!isEof)
									isEof = Parse(experIter, valueOp, ref nextValueOp, ref isEnd);
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
			else if (curCh == '(')
			{
				throw new FormatException();
			}
			else if (curCh == ')')
			{
				if (bracketStack.Count <= 0)
					throw new FormatException();
				BracketType type = bracketStack.Pop();
				if (type == BracketType.FUNCTION)
					isFunBracketEnd = true;
				isEof = !experIter.MoveNext();
				return isEof;
			}
			else if (curCh == ',')
			{
				isEnd = true;
				op = null;
			}
			else
			{
				isEof = ParseItem(experIter, OperatorOrder.NEED_OPERATOR, ref op);
				if (op is TwoObjOp)
					((TwoObjOp)op).LhsItem = prevOp;
			}
			return isEof;
		}

		private bool Parse(CharEnumerator experIter, OperatorOrder opOrder, ref IOperator op)
		{
			op = null;
			if (!SkipSpace(experIter))
				return true;

			bool isEof = false;
			if (IsOp(experIter.Current))
			{
				if (experIter.Current == '-')
				{
					isEof = !experIter.MoveNext();
					IOperator valueOp = null;
					if (!isEof)
						isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
					if (valueOp == null)
						throw new FormatException();
					op = new NegOp(valueOp);
				}
				else if (experIter.Current == '+')
				{
					isEof = !experIter.MoveNext();
					if (isEof)
						throw new FormatException();
					isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref op);
				}
				else
					throw new FormatException();
			}
			else if (experIter.Current == '(')
			{
				isEof = !experIter.MoveNext();
				if (isEof)
					throw new FormatException();

				if (!SkipSpace(experIter))
					throw new FormatException();

				if (opOrder == OperatorOrder.NEED_PARAM)
					bracketStack.Push(BracketType.FUNCTION);
				else
					bracketStack.Push(BracketType.GENERAL);
				isFunBracketEnd = false;
				isEof = Parse(experIter, OperatorOrder.NONE, ref op);
			}
			else if (experIter.Current == ')')
			{
				if (bracketStack.Count <= 0)
					throw new FormatException();
				BracketType type = bracketStack.Pop();
				if (type == BracketType.FUNCTION)
					isFunBracketEnd = true;
				isEof = !experIter.MoveNext();
				return isEof;
			}
			else
			{
				isEof = ParseItem(experIter, OperatorOrder.NEED_VALUE, ref op);
			}

			bool isEnd = false;
			while (!isEof && !isEnd && !isFunBracketEnd
				&& (opOrder == OperatorOrder.NONE || opOrder == OperatorOrder.NEED_PARAM)
				&& experIter.Current != ',')
			{
				IOperator nextValueOp = null;
				isEof = Parse(experIter, op, ref nextValueOp, ref isEnd);
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
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
				op = new MulOp(null, valueOp);
			}
			else if (item.Equals("DIFFERENCE"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
				IOperator nextValueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref nextValueOp);
				op = new SubOp(valueOp, nextValueOp);
			}
			else if (item.Equals("SUM"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
				IOperator nextValueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref nextValueOp);
				op = new AddOp(valueOp, nextValueOp);
			}
			else if (item.Equals("DIV"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
				IOperator nextValueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref nextValueOp);
				op = new DivOp(valueOp, nextValueOp);
			}
			else if (item.Equals("TANF"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
				IOperator nextValueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref nextValueOp);
				op = new TanfOp(valueOp, nextValueOp);
			}
			else if (item.Equals("MIN"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_PARAM, ref valueOp);
				if (isEof || experIter.Current != ',' || !experIter.MoveNext())
					throw new FormatException();
				IOperator nextValueOp = null;
				isEof = Parse(experIter, OperatorOrder.NONE, ref nextValueOp);
				isFunBracketEnd = false;
				op = new MinOp(valueOp, nextValueOp);
			}
			else if (item.Equals("MAX"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_PARAM, ref valueOp);
				if (isEof || experIter.Current != ',' || !experIter.MoveNext())
					throw new FormatException();
				IOperator nextValueOp = null;
				isEof = Parse(experIter, OperatorOrder.NONE, ref nextValueOp);
				isFunBracketEnd = false;
				op = new MaxOp(valueOp, nextValueOp);
			}
			else if (item.Equals("COS"))
			{
				CosOp cosOp = new CosOp();
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_PARAM, ref valueOp);
				isFunBracketEnd = false;
				cosOp.Item = valueOp;
				op = cosOp;
				return isEof;
			}
			else if (item.Equals("SIN"))
			{
				SinOp sinOp = new SinOp();
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_PARAM, ref valueOp);
				isFunBracketEnd = false;
				sinOp.Item = valueOp;
				op = sinOp;
				return isEof;
			}
			else if (item.Equals("SQRT"))
			{
				SqrtOp sqrtOp = new SqrtOp();
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_PARAM, ref valueOp);
				isFunBracketEnd = false;
				sqrtOp.Item = valueOp;
				op = sqrtOp;
				return isEof;
			}
			else if (item.Equals("INT"))
			{
				IntOp intOp = new IntOp();
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_PARAM, ref valueOp);
				isFunBracketEnd = false;
				intOp.Item = valueOp;
				op = intOp;
				return isEof;
			}
			else if (item.Equals("PARAM"))
			{
				ParamOp paramOp = new ParamOp(this);
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
				paramOp.Item = valueOp;
				op = paramOp;
				return isEof;
			}
			else if (item.Equals("IPARAM"))
			{
				IParamOp paramOp = new IParamOp(this);
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
				paramOp.Item = valueOp;
				op = paramOp;
				return isEof;
			}
			else if (item.Equals("DESIGN"))
			{
				isEof = !SkipSpace(experIter);
				if (isEof)
					throw new FormatException();

				sb = new StringBuilder();
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

				item = sb.ToString();
				if (item.Equals("PARAM"))
				{
					DParamOp paramOp = new DParamOp(this);
					IOperator valueOp = null;
					isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
					paramOp.Item = valueOp;
					op = paramOp;
					return isEof;
				}
				else
					throw new FormatException();
			}
			else if (item.Equals("TWICE"))
			{
				IOperator valueOp = null;
				isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
				op = new TwiceOp(valueOp);
			}
			else if (item.Equals("ATTRIB"))
			{
				isEof = !SkipSpace(experIter);
				if (isEof)
					throw new FormatException();

				sb = new StringBuilder();
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

				item = sb.ToString();

				if (item.Equals("RPRO"))
				{
					isEof = !SkipSpace(experIter);
					if (isEof)
						throw new FormatException();

					sb = new StringBuilder();
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

					item = sb.ToString();

					DbExperOp experOp = new DbExperOp(this);
					experOp.DbExper = DbExpression.Parse("ATTRIB RPRO " + item);
					op = experOp;
				}
				else
				{
					DbAttribute attr = DbAttribute.GetDbAttribute(item);
					if (attr == null)
						throw new FormatException();

					while (!isEof && Char.IsWhiteSpace(experIter.Current))
					{
						isEof = !experIter.MoveNext();
					}

					if (isEof || experIter.Current == '[')
					{
						AttribArrayOp paramOp = new AttribArrayOp(this);
						paramOp.Attr = attr;
						IOperator valueOp = null;
						isEof = Parse(experIter, OperatorOrder.NEED_VALUE, ref valueOp);
						paramOp.Item = valueOp;
						op = paramOp;
					}
					else
					{
						AttribOp paramOp = new AttribOp(this);
						paramOp.Attr = attr;
						op = paramOp;
					}
				}
			}
			else if (item.Equals("PL") || item.Equals("PA"))
			{
				isEof = !SkipSpace(experIter);
				if (isEof)
					throw new FormatException();

				sb = new StringBuilder();
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

				DbAttribute attr = item.Equals("PL") ? DbAttributeInstance.LOD : DbAttributeInstance.AOD;
				item = sb.ToString();
				if (!item.Equals("OD"))
					throw new FormatException();

				AttribOp paramOp = new AttribOp(this);
				paramOp.Attr = attr;
				op = paramOp;
			}
			else if (item.Equals("PLOD"))
			{
				AttribOp paramOp = new AttribOp(this);
				paramOp.Attr = DbAttributeInstance.LOD;
				op = paramOp;
			}
			else if (item.Equals("PAOD"))
			{
				AttribOp paramOp = new AttribOp(this);
				paramOp.Attr = DbAttributeInstance.AOD;
				op = paramOp;
			}
			else
				throw new FormatException();

			return isEof;
		}
	}

	interface IOperator
	{
		double Eval();
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
		public virtual double Eval()
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

		public virtual double Eval()
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

		public override double Eval()
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

		public override double Eval()
		{
			return LhsItem.Eval() - RhsItem.Eval();
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

		public override double Eval()
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

		public override double Eval()
		{
			return LhsItem.Eval() / RhsItem.Eval();
		}
	}

	class TanfOp : TwoObjOp
	{
		public TanfOp()
		{

		}

		public TanfOp(IOperator lhs, IOperator rhs)
			: base(lhs, rhs)
		{

		}

		public override double Eval()
		{
			return LhsItem.Eval() * Math.Tan(RhsItem.Eval() * Math.PI / 180.0 / 2.0);
		}
	}

	class MinOp : TwoObjOp
	{
		public MinOp()
		{

		}

		public MinOp(IOperator lhs, IOperator rhs)
			: base(lhs, rhs)
		{

		}

		public override double Eval()
		{
			return Math.Min(LhsItem.Eval(), RhsItem.Eval());
		}
	}

	class MaxOp : TwoObjOp
	{
		public MaxOp()
		{

		}

		public MaxOp(IOperator lhs, IOperator rhs)
			: base(lhs, rhs)
		{

		}

		public override double Eval()
		{
			return Math.Max(LhsItem.Eval(), RhsItem.Eval());
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

		public override double Eval()
		{
			return -Item.Eval();
		}
	}

	class IntOp : OneObjOp
	{
		public IntOp()
		{

		}

		public IntOp(IOperator item)
			: base(item)
		{

		}

		public override double Eval()
		{
			return (int)Item.Eval();
		}
	}

	class CosOp : OneObjOp
	{
		public CosOp()
		{

		}

		public CosOp(IOperator item)
			: base(item)
		{

		}

		public override double Eval()
		{
			return Math.Cos(Item.Eval());
		}
	}

	class SinOp : OneObjOp
	{
		public SinOp()
		{

		}

		public SinOp(IOperator item)
			: base(item)
		{

		}

		public override double Eval()
		{
			return Math.Sin(Item.Eval());
		}
	}

	class SqrtOp : OneObjOp
	{
		public SqrtOp()
		{

		}

		public SqrtOp(IOperator item)
			: base(item)
		{

		}

		public override double Eval()
		{
			return Math.Sqrt(Item.Eval());
		}
	}

	class TwiceOp : OneObjOp
	{
		public TwiceOp()
		{

		}

		public TwiceOp(IOperator item)
			: base(item)
		{

		}

		public override double Eval()
		{
			return Item.Eval() * 2.0;
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

		public override double Eval()
		{
			DbElement spref = Exper.ModelElement.GetElement(DbAttributeInstance.SPRE);
			DbElement cate = spref.GetElement(DbAttributeInstance.CATR);
			double[] para = cate.GetDoubleArray(DbAttributeInstance.PARA);
			if (para == null || para.Length <= 0)
				return 0.0;
			return para[(int)Item.Eval() - 1];
		}
	}

	class DParamOp : EleOp
	{
		public DParamOp(Experssion exper)
			: base(exper)
		{
		}

		public override double Eval()
		{
			double[] deps = Exper.ModelElement.GetDoubleArray(DbAttributeInstance.DESP);
			if (deps == null || deps.Length <= 0)
				return 0.0;
			return deps[(int)Item.Eval() - 1];
		}
	}

	class AttribArrayOp : EleOp
	{
		public DbAttribute Attr { get; set; }

		public AttribArrayOp(Experssion exper)
			: base(exper)
		{
		}

		public void SetAttr(string name)
		{
			Attr = DbAttribute.GetDbAttribute(name);
		}

		public override double Eval()
		{
			double[] attr = Exper.ModelElement.GetDoubleArray(Attr);
			if (attr == null || attr.Length <= 0)
				return 0.0;
			return attr[(int)Item.Eval() - 1];
		}
	}

	class AttribOp : IOperator
	{
		public Experssion Exper { get; set; }
		public DbAttribute Attr { get; set; }

		public AttribOp(Experssion exper)
		{
			Exper = exper;
		}

		public void SetAttr(string name)
		{
			Attr = DbAttribute.GetDbAttribute(name);
		}

		public double Eval()
		{
			return Exper.ModelElement.GetDouble(Attr);
		}
	}

	class DbExperOp : IOperator
	{
		public Experssion Exper { get; set; }
		public DbExpression DbExper { get; set; }
		public DbExperOp(Experssion exper)
		{
			Exper = exper;
		}

		public double Eval()
		{
			return Exper.ModelElement.EvaluateDouble(DbExper, DbAttributeUnit.DIST);
		}
	}

	class TwoAttribOp : EleOp
	{
		public DbAttribute Attr1 { get; set; }
		public DbAttribute Attr2 { get; set; }

		public TwoAttribOp(Experssion exper)
			: base(exper)
		{
		}

		public override double Eval()
		{
			DbElement ele = Exper.ModelElement.GetElement(Attr1);
			double[] attr = ele.GetDoubleArray(Attr2);
			if (attr == null || attr.Length <= 0)
				return 0.0;
			return attr[(int)Item.Eval() - 1];
		}
	}

	class IParamOp : EleOp
	{
		public IParamOp(Experssion exper)
			: base(exper)
		{
		}

		public override double Eval()
		{
			//DbElement ispe = Exper.ModelElement.GetElement(DbAttributeInstance.ISPE);
			//if (ispe == null || !ispe.IsValid)
			//	return 0.0;

			//Double[] ipar = ispe.GetDoubleArray(DbAttributeInstance.PARA);
			double[] ipar = Exper.ModelElement.GetDoubleArray(DbAttributeInstance.IPAR);
			int idx = (int)Item.Eval();
			if (ipar == null || ipar.Length < idx)
				return 0.0;
			return ipar[idx - 1];
		}
	}

	class HeightOp : EleOp
	{
		public HeightOp(Experssion exper) : base(exper)
		{

		}

		public override double Eval()
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

		public override double Eval()
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

		public override double Eval()
		{
			return Exper.ModelElement.GetDouble(DbAttributeInstance.RADI);
		}
	}
}
