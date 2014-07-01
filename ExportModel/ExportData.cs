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
				util.init(fileDlg.FileName);

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

		private void ExportBranch(DbElement branchEle)
		{
			DbElement ele = branchEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				DbElement specEle = null;
				if (!IsReadableEle(ele)
					|| (specEle = ele.GetElement(DbAttributeInstance.SPRE)) == null
					|| !IsReadableEle(specEle))
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
				if ((gmEle = cateEle.GetElement(DbAttributeInstance.GMRE)) == null || !IsReadableEle(cateEle))
				{
					ele = ele.Next();
					continue;
				}

				DbElement gEle = gmEle.FirstMember();
				while (gEle != null && gEle.IsValid)
				{
					if (IsReadableEle(gEle))
					{
						if (gEle.GetElementType() == DbElementTypeInstance.SCYLINDER)
						{
							string expr = gEle.GetAsString(DbAttributeInstance.PAXI);
							AddExpr(expr);
							PointVector paxi = EvalDirection.Eval(ele, expr);

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
							Direction dir = ori.AbsoluteDirection(paxi.Direction);
							Position pos = ori.AbsolutePosition(paxi.Position);

							Cylinder cyl = new Cylinder();
							cyl.Org = new Point(pos)
								.MoveBy(ele.GetPosition(DbAttributeInstance.POS))
								.MoveBy(dir, pdia);
							cyl.Height = new Point(dir).Mul(phei);
							cyl.Radius = pdia / 2.0;
							session.Save(cyl);
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
			using (StreamWriter sw = new StreamWriter(@"g:\temp\expr.txt"))
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
