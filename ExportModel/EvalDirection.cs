using Aveva.Pdms.Database;
using Aveva.Pdms.Geometry;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ExportModel
{
	class EvalDirection
	{
		private static char[] flags = new char[] { 'X', 'Y', 'Z' };
		public static AxisDir Eval(DbElement modelEle, string exper)
		{
			exper = exper.Trim().ToUpper();
			if (exper.IndexOfAny(flags) >= 0)
			{
				return new AxisDir(Position.Create(), ParseExperDir(exper, modelEle));
			}
			else if (exper.Contains('P'))
			{
				bool isNeg = false;
				if (exper[0] == '-')
				{
					isNeg = true;
					exper = exper.Substring(1).Trim();
				}

				exper = exper.Substring(exper[1] == 'P' ? 2 : 1);

				int num = int.Parse(exper);
				return MakeDirection(num, isNeg, modelEle);
			}
			else
			{
				return new AxisDir(Position.Create(), Direction.Create(exper));
			}
		}

		private static AxisDir MakeDirection(int num, bool isNeg, DbElement modelEle)
		{
			DbElement cate = modelEle.GetElement(DbAttributeInstance.SPRE).GetElement(DbAttributeInstance.CATR);
			DbElement ptre = cate.GetElement(DbAttributeInstance.PTRE);
			DbElement pt = ptre.FirstMember();
			while (pt != null && pt.IsValid)
			{
				if (num != pt.GetInteger(DbAttributeInstance.NUMB))
				{
					pt = pt.Next();
					continue;
				}

				DbElementType type = pt.GetElementType();
				if (type == DbElementTypeInstance.PTAXIS)
					return MakeAxialDirection(num, isNeg, pt, modelEle);
				else if (type == DbElementTypeInstance.PTCAR)
					return MakeCartesianDirection(num, isNeg, pt, modelEle);
				else if (type == DbElementTypeInstance.PTMIX)
					return MakeMixedDirection(num, isNeg, pt, modelEle);
				else if (type == DbElementTypeInstance.PTPOS)
					return MakePositionTypeDirection(num, isNeg, pt, modelEle);
				pt = pt.Next();
			}
			return null;
		}

		private static AxisDir MakeAxialDirection(int num, bool isNeg, DbElement pnt, DbElement modelEle)
		{
			Direction dir = ParseExperDir(pnt.GetAsString(DbAttributeInstance.PAXI), modelEle);
			double offset = EvalDouble(pnt.GetAsString(DbAttributeInstance.PDIS), modelEle);
			Position pos = Position.Create();
			pos.MoveBy(dir, offset);
			if (isNeg)
				dir = dir.Opposite();
			return new AxisDir(pos, dir);
		}

		private static double EvalDouble(string strExper, DbElement modelEle)
		{
			Experssion exper = new Experssion(strExper);
			return exper.Eval(modelEle);
		}

		private static AxisDir MakeCartesianDirection(int num, bool isNeg, DbElement pnt, DbElement modelEle)
		{
			Direction dir = ParseExperDir(pnt.GetAsString(DbAttributeInstance.PTCD), modelEle);
			double x = EvalDouble(pnt.GetAsString(DbAttributeInstance.PX), modelEle);
			double y = EvalDouble(pnt.GetAsString(DbAttributeInstance.PY), modelEle);
			double z = EvalDouble(pnt.GetAsString(DbAttributeInstance.PZ), modelEle);
			if (isNeg)
				dir = dir.Opposite();
			return new AxisDir(Position.Create(x, y, z), dir);
		}

		private static AxisDir MakeMixedDirection(int num, bool isNeg, DbElement pnt, DbElement modelEle)
		{
			Direction dir = ParseExperDir(pnt.GetAsString(DbAttributeInstance.PAXI), modelEle);
			double x = EvalDouble(pnt.GetAsString(DbAttributeInstance.PX), modelEle);
			double y = EvalDouble(pnt.GetAsString(DbAttributeInstance.PY), modelEle);
			double z = EvalDouble(pnt.GetAsString(DbAttributeInstance.PZ), modelEle);
			if (isNeg)
				dir = dir.Opposite();
			return new AxisDir(Position.Create(x, y, z), dir);
		}

		private static AxisDir MakePositionTypeDirection(int num, bool isNeg, DbElement pnt, DbElement modelEle)
		{
			Direction dir = ParseExperDir(pnt.GetAsString(DbAttributeInstance.PTCD), modelEle);
			AxisDir pos = Eval(modelEle, pnt.GetAsString(DbAttributeInstance.PTCPOS));
			if (isNeg)
				dir = dir.Opposite();
			return new AxisDir(pos.Pos, dir);
		}

		private static Direction ParseExperDir(string exper, DbElement modelEle)
		{
			if (exper.IndexOfAny(flags) < 0 && exper.Contains("P"))
			{
				AxisDir dir = Eval(modelEle, exper);
				return dir.Dir;
			}

			StringBuilder sb = new StringBuilder();
			int preIdx = 0, nextIdx;
			while ((nextIdx = exper.IndexOfAny(flags, preIdx)) >= 0)
			{
				string item = exper.Substring(preIdx, nextIdx - preIdx).Trim();
				if (item.Length > 0)
				{
					if (item[item.Length-1] == '-')
					{
						item = item.Substring(0, item.Length-1).Trim();
						if (item.Length > 0)
							sb.Append(EvalDouble(item, modelEle).ToString());
						sb.Append('-');
					}
					else
						sb.Append(EvalDouble(item, modelEle).ToString());
				}
				sb.Append(exper[nextIdx]);
				preIdx = nextIdx + 1;
			}

			if (sb.Length <= 0)
				sb.Append(exper);
			return Direction.Create(sb.ToString());
		}
	}
}
