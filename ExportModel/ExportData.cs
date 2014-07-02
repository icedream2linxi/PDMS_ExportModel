using Aveva.Pdms.Database;
using Aveva.Pdms.Geometry;
using Aveva.Pdms.Shared;
using DbModel;
using NHibernate;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace ExportModel
{
	class ExportData
	{
		private string dbPath;
		private string dbName;
		private HashSet<string> exprSet = new HashSet<string>();
		private Dictionary<DbAttribute, Dictionary<string, Experssion>> experMap = new Dictionary<DbAttribute, Dictionary<string, Experssion>>();
		private ISession session = null;

		public void Export()
		{
			if (CurrentElement.Element.DbType != DbType.Design)
			{
				MessageBox.Show("仅支持在Design模块中导出数据。\n", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
				return;
			}

			SaveFileDialog fileDlg = new SaveFileDialog();
			fileDlg.InitialDirectory = string.IsNullOrEmpty(dbPath) ? "c:\\" : dbPath;
			fileDlg.Filter = "sqlite db files (*.db)|*.db";
			fileDlg.FilterIndex = 0;
			fileDlg.RestoreDirectory = true;
			fileDlg.AddExtension = true;
			fileDlg.FileName = string.IsNullOrEmpty(dbName) ? Project.CurrentProject.Name : dbName;
			if (fileDlg.ShowDialog() != DialogResult.OK)
			{
				return;
			}
			dbPath = fileDlg.FileName;
			dbPath.Trim();
			dbName = dbPath;
			dbName.Replace('/', '\\');
			dbName = dbName.Substring(dbName.LastIndexOf('\\') + 1, dbName.Length - dbName.LastIndexOf('\\') - 1);

			using (Util util = new Util())
			{
				util.init(fileDlg.FileName, true);

				using (session = util.SessionFactory.OpenSession())
				using (ITransaction tx = session.BeginTransaction())
				{
					Export(CurrentElement.Element);

					tx.Commit();
				}
			}

			Console.WriteLine("Export model finish!!!");
		}

		private void Export(DbElement ele)
		{
			if (ele.GetElementType() == DbElementTypeInstance.WORLD)
				ExportWorld(ele);
			else if (ele.GetElementType() == DbElementTypeInstance.SITE)
				ExportSite(ele);
			else if (ele.GetElementType() == DbElementTypeInstance.ZONE)
				ExportZone(ele);
			else if (ele.GetElementType() == DbElementTypeInstance.PIPE)
				ExportPipe(ele);
			else if (ele.GetElementType() == DbElementTypeInstance.EQUIPMENT)
				ExportEquip(ele);
			else if (ele.GetElementType() == DbElementTypeInstance.BRANCH)
				ExportBranch(ele);
			SaveExpr();
		}

		private bool IsReadableEle(DbElement ele)
		{
			return ele.IsValid && !ele.IsNull && !ele.IsDeleted;
		}

		private void ExportWorld(DbElement worldEle)
		{
			DbElement ele = worldEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				if (IsReadableEle(ele) && ele.GetElementType() == DbElementTypeInstance.SITE)
					ExportSite(ele);
				ele = ele.Next();
			}
		}

		private void ExportSite(DbElement siteEle)
		{
			DbElement ele = siteEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				if (IsReadableEle(ele) && ele.GetElementType() == DbElementTypeInstance.ZONE)
					ExportZone(ele);
				ele = ele.Next();
			}
		}

		private void ExportZone(DbElement zoneEle)
		{
			DbElement ele = zoneEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				if (IsReadableEle(ele))
				{
					if (ele.GetElementType() == DbElementTypeInstance.PIPE)
						ExportPipe(ele);
					else if (ele.GetElementType() == DbElementTypeInstance.EQUIPMENT)
						ExportEquip(ele);
				}
				ele = ele.Next();
			}
		}

		private void ExportPipe(DbElement pipeEle)
		{
			DbElement ele = pipeEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				if (IsReadableEle(ele) && ele.GetElementType() == DbElementTypeInstance.BRANCH)
					ExportBranch(ele);
				ele = ele.Next();
			}
		}

		private Experssion GetExper(DbElement gEle, DbAttribute attr)
		{
			Dictionary<string, Experssion> map = null;
			if (!experMap.TryGetValue(attr, out map))
			{
				map = new Dictionary<string, Experssion>();
				experMap.Add(attr, map);
			}

			Experssion exper = null;
			if (!map.TryGetValue(gEle.GetAsString(DbAttributeInstance.NAME), out exper))
			{
				exper = new Experssion(gEle.GetAsString(attr));
				map.Add(gEle.GetAsString(DbAttributeInstance.NAME), exper);
			}

			return exper;
		}

		private void ExportTube(DbElement tubeEle)
		{
			double ltLength = tubeEle.GetDouble(DbAttributeInstance.ITLE);
			double lbore = tubeEle.GetDoubleArray(DbAttributeInstance.PARA)[1];

			DbElement prevEle = tubeEle.Previous;
			int leave = 1;
			if (prevEle == null || !prevEle.IsValid)
			{
				prevEle = tubeEle.Next();
				leave = prevEle.GetInteger(DbAttributeInstance.ARRI);
			}
			else
				leave = prevEle.GetInteger(DbAttributeInstance.LEAV);
			AxisDir ptax = EvalDirection.Eval(prevEle, "P" + leave);

			Aveva.Pdms.Geometry.Orientation ori = prevEle.GetOrientation(DbAttributeInstance.ORI);
			Direction dir = ori.AbsoluteDirection(ptax.Dir);
			Position pos = Position.Create();
			double dist = ptax.Pos.Distance(pos);
			if (dist > 0.00001)
			{
				pos.MoveBy(ori.AbsoluteDirection(Direction.Create(ptax.Pos)), dist);
			}

			Cylinder cyl = new Cylinder();
			cyl.Org = new Point(pos)
				.MoveBy(prevEle.GetPosition(DbAttributeInstance.POS));
			cyl.Height = new Point(dir).Mul(ltLength);
			cyl.Radius = lbore / 2.0;
			session.Save(cyl);
		}

		private void ExportBranch(DbElement branchEle)
		{
			DbElement ele = branchEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				if (!IsReadableEle(ele))
				{
					ele = ele.Next();
					continue;
				}

				if (ele.GetElementType() == DbElementTypeInstance.TUBING)
				{
					ExportTube(ele);
					ele = ele.Next();
					continue;
				}

				DbElement specEle = null;
				if ((specEle = ele.GetElement(DbAttributeInstance.SPRE)) == null
					|| !IsReadableEle(specEle)
					|| ele.GetElementType() == DbElementTypeInstance.ATTACHMENT
					)
				{
					ele = ele.Next();
					continue;
				}

				DbElement cateEle = null;
				if ((cateEle = specEle.GetElement(DbAttributeInstance.CATR)) == null || !IsReadableEle(cateEle))
				{
					ele = ele.Next();
					continue;
				}

				DbElement gmEle = null;
				if ((gmEle = cateEle.GetElement(DbAttributeInstance.GMRE)) == null || !IsReadableEle(gmEle))
				{
					ele = ele.Next();
					continue;
				}

				DbElement gEle = gmEle.FirstMember();
				while (gEle != null && gEle.IsValid)
				{
					if (IsReadableEle(gEle) && gEle.GetBool(DbAttributeInstance.TUFL))
					{
						if (gEle.GetElementType() == DbElementTypeInstance.SCYLINDER)
						{
							string expr = gEle.GetAsString(DbAttributeInstance.PAXI);
							AddExpr(expr);
							AxisDir paxi = EvalDirection.Eval(ele, expr);

							expr = gEle.GetAsString(DbAttributeInstance.PHEI);
							AddExpr(expr);
							double phei = GetExper(gEle, DbAttributeInstance.PHEI).Eval(ele);

							expr = gEle.GetAsString(DbAttributeInstance.PDIA);
							AddExpr(expr);
							double pdia = GetExper(gEle, DbAttributeInstance.PDIA).Eval(ele);

							expr = gEle.GetAsString(DbAttributeInstance.PDIS);
							AddExpr(expr);
							double pdis = GetExper(gEle, DbAttributeInstance.PDIS).Eval(ele);

							Aveva.Pdms.Geometry.Orientation ori = ele.GetOrientation(DbAttributeInstance.ORI);
							Direction dir = ori.AbsoluteDirection(paxi.Dir);
							Position pos = Position.Create();
							double dist = paxi.Pos.Distance(pos);
							if (dist > 0.00001)
							{
								pos.MoveBy(ori.AbsoluteDirection(Direction.Create(paxi.Pos)), dist);
							}

							Cylinder cyl = new Cylinder();
							cyl.Org = new Point(pos)
								.MoveBy(ele.GetPosition(DbAttributeInstance.POS))
								.MoveBy(dir, pdis);
							cyl.Height = new Point(dir).Mul(phei);
							cyl.Radius = pdia / 2.0;
							session.Save(cyl);
						}
						else if (gEle.GetElementType() == DbElementTypeInstance.LCYLINDER)
						{
							string expr = gEle.GetAsString(DbAttributeInstance.PAXI);
							AddExpr(expr);
							AxisDir paxi = EvalDirection.Eval(ele, expr);

							expr = gEle.GetAsString(DbAttributeInstance.PDIA);
							AddExpr(expr);
							double pdia = GetExper(gEle, DbAttributeInstance.PDIA).Eval(ele);

							expr = gEle.GetAsString(DbAttributeInstance.PBDI);
							AddExpr(expr);
							double pbdi = GetExper(gEle, DbAttributeInstance.PBDI).Eval(ele);

							expr = gEle.GetAsString(DbAttributeInstance.PTDI);
							AddExpr(expr);
							double ptdi = GetExper(gEle, DbAttributeInstance.PTDI).Eval(ele);

							Aveva.Pdms.Geometry.Orientation ori = ele.GetOrientation(DbAttributeInstance.ORI);
							Direction dir = ori.AbsoluteDirection(paxi.Dir);
							Position pos = Position.Create();
							double dist = paxi.Pos.Distance(pos);
							if (dist > 0.00001)
							{
								pos.MoveBy(ori.AbsoluteDirection(Direction.Create(paxi.Pos)), dist);
							}

							Cylinder cyl = new Cylinder();
							cyl.Org = new Point(pos)
								.MoveBy(ele.GetPosition(DbAttributeInstance.POS))
								.MoveBy(dir, pbdi);
							cyl.Height = new Point(dir).Mul(ptdi - pbdi);
							cyl.Radius = pdia / 2.0;
							session.Save(cyl);
						}
						else if (gEle.GetElementType() == DbElementTypeInstance.SBOX)
						{
							string exper = gEle.GetString(DbAttributeInstance.PXLE);
							AddExpr(exper);
							double pxlen = GetExper(gEle, DbAttributeInstance.PXLE).Eval(ele);

							exper = gEle.GetString(DbAttributeInstance.PYLE);
							AddExpr(exper);
							double pylen = GetExper(gEle, DbAttributeInstance.PYLE).Eval(ele);

							exper = gEle.GetString(DbAttributeInstance.PZLE);
							AddExpr(exper);
							double pzlen = GetExper(gEle, DbAttributeInstance.PZLE).Eval(ele);

							exper = gEle.GetString(DbAttributeInstance.PX);
							AddExpr(exper);
							double px = GetExper(gEle, DbAttributeInstance.PX).Eval(ele);

							exper = gEle.GetString(DbAttributeInstance.PY);
							AddExpr(exper);
							double py = GetExper(gEle, DbAttributeInstance.PY).Eval(ele);

							exper = gEle.GetString(DbAttributeInstance.PZ);
							AddExpr(exper);
							double pz = GetExper(gEle, DbAttributeInstance.PZ).Eval(ele);
							
							Aveva.Pdms.Geometry.Orientation ori = ele.GetOrientation(DbAttributeInstance.ORI);
							Point xlen = new Point(ori.AbsoluteDirection(Direction.Create(Axis.EAST)));
							Point ylen = new Point(ori.AbsoluteDirection(Direction.Create(Axis.NORTH)));
							Point zlen = new Point(ori.AbsoluteDirection(Direction.Create(Axis.UP)));

							Point pos = new Point(ele.GetPosition(DbAttributeInstance.POS));
							pos.MoveBy(xlen, px - pxlen / 2.0).MoveBy(ylen, py - pylen / 2.0).MoveBy(zlen, pz - pzlen / 2.0);
							xlen.Mul(pxlen);
							ylen.Mul(pylen);
							zlen.Mul(pzlen);

							Box box = new Box();
							box.Org = pos;
							box.XLen = xlen;
							box.YLen = ylen;
							box.ZLen = zlen;
							session.Save(box);
						}
					}

					gEle = gEle.Next();
				}

				ele = ele.Next();
			}
		}

		private void ExportEquip(DbElement equipEle)
		{

		}

		private void AddExpr(string expr)
		{
			double db;
			if (expr == null || Double.TryParse(expr, out db))
				return;
			exprSet.Add(expr);
		}

		private void SaveExpr()
		{
			using (StreamWriter sw = new StreamWriter(@"f:\temp\expr.txt"))
			{
				string[] arr = new string[exprSet.Count];
				exprSet.CopyTo(arr);
				Array.Sort(arr);
				foreach (string str in arr)
				{
					sw.WriteLine(str);
				}
				sw.Flush();
			}
		}
	}
}
