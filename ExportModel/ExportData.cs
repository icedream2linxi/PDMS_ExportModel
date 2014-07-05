﻿using Aveva.Pdms.Database;
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
			fileDlg.InitialDirectory = string.IsNullOrEmpty(dbPath) ? "d:\\" : dbPath;
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
			Direction dir = null;

			DbElement prevEle = tubeEle.Previous;
			if (prevEle == null || !prevEle.IsValid)
			{
				DbElement branchEle = tubeEle.Owner;
				dir = branchEle.GetDirection(DbAttributeInstance.HDIR);
			}
			else
			{
				int leave = prevEle.GetInteger(DbAttributeInstance.LEAV);
				AxisDir ptax = EvalDirection.Eval(prevEle, "P" + leave);

				Aveva.Pdms.Geometry.Orientation ori = prevEle.GetOrientation(DbAttributeInstance.ORI);
				dir = ori.AbsoluteDirection(ptax.Dir);
			}

			Position pos = tubeEle.GetPosition(DbAttributeInstance.ITPS);
			pos.MoveBy(dir, -ltLength / 2);

			Cylinder cyl = new Cylinder();
			cyl.Org = new Point(pos);
			cyl.Height = new Point(dir).Mul(ltLength);
			cyl.Radius = lbore / 2.0;
			session.Save(cyl);
		}

		private void ExportBranch(DbElement branchEle)
		{
			DbElement ele = branchEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				if (IsReadableEle(ele))
					ExportPipeItem(ele);
				ele = ele.Next();
			}
		}

		private void ExportPipeItem(DbElement ele)
		{
			if (ele.GetElementType() == DbElementTypeInstance.TUBING)
			{
				ExportTube(ele);
				return;
			}

			DbElement specEle = null;
			if ((specEle = ele.GetElement(DbAttributeInstance.SPRE)) == null
				|| !IsReadableEle(specEle)
				|| ele.GetElementType() == DbElementTypeInstance.ATTACHMENT
				)
			{
				return; ;
			}

			DbElement cateEle = null;
			if ((cateEle = specEle.GetElement(DbAttributeInstance.CATR)) == null || !IsReadableEle(cateEle))
			{
				return;
			}

			DbElement gmEle = null;
			if ((gmEle = cateEle.GetElement(DbAttributeInstance.GMRE)) == null || !IsReadableEle(gmEle))
			{
				return;
			}

			DbElement gEle = gmEle.FirstMember();
			while (gEle != null && gEle.IsValid)
			{
				if (IsReadableEle(gEle) && gEle.GetBool(DbAttributeInstance.TUFL))
				{
					if (gEle.GetElementType() == DbElementTypeInstance.SCYLINDER)
					{
						string expr = gEle.GetAsString(DbAttributeInstance.PAXI);
						AxisDir paxi = EvalDirection.Eval(ele, expr);

						double phei = GetExper(gEle, DbAttributeInstance.PHEI).Eval(ele);
						double pdia = GetExper(gEle, DbAttributeInstance.PDIA).Eval(ele);
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
						AxisDir paxi = EvalDirection.Eval(ele, expr);

						double pdia = GetExper(gEle, DbAttributeInstance.PDIA).Eval(ele);
						double pbdi = GetExper(gEle, DbAttributeInstance.PBDI).Eval(ele);
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
						double pxlen = GetExper(gEle, DbAttributeInstance.PXLE).Eval(ele);
						double pylen = GetExper(gEle, DbAttributeInstance.PYLE).Eval(ele);
						double pzlen = GetExper(gEle, DbAttributeInstance.PZLE).Eval(ele);
						double px = GetExper(gEle, DbAttributeInstance.PX).Eval(ele);
						double py = GetExper(gEle, DbAttributeInstance.PY).Eval(ele);
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
					else if (gEle.GetElementType() == DbElementTypeInstance.SCTORUS)
					{
						string expr = gEle.GetAsString(DbAttributeInstance.PAAX);
						AxisDir paax = EvalDirection.Eval(ele, expr);

						expr = gEle.GetAsString(DbAttributeInstance.PBAX);
						AxisDir pbax = EvalDirection.Eval(ele, expr);

						double pdia = GetExper(gEle, DbAttributeInstance.PDIA).Eval(ele);

						Aveva.Pdms.Geometry.Orientation ori = ele.GetOrientation(DbAttributeInstance.ORI);
						CircularTorus ct = new CircularTorus();
						Direction normal = null;
						if (!paax.Dir.IsParallel(pbax.Dir))
							normal = paax.Dir.Orthogonal(pbax.Dir);
						else
							normal = paax.Dir.Orthogonal(Direction.Create(paax.Pos, pbax.Pos));
						ct.Normal = new Point(ori.AbsoluteDirection(normal));
						Position pos = Position.Create();
						double dist = pbax.Pos.Distance(pos);
						if (dist > 0.00001)
						{
							pos.MoveBy(ori.AbsoluteDirection(Direction.Create(pbax.Pos)), dist);
						}
						ct.StartPnt = new Point(pos).MoveBy(ele.GetPosition(DbAttributeInstance.POS));
						ct.Radius = pdia / 2.0;

						double mRadius = 0.0;
						if (!paax.Dir.IsParallel(pbax.Dir))
						{
							double ang = paax.Dir.Angle(pbax.Dir) * Math.PI / 180.0;
							ct.Angle = Math.PI - ang;
							ang /= 2.0;
							double len = paax.Pos.Distance(pbax.Pos) / 2;
							mRadius = len / Math.Sin(ang) * Math.Tan(ang);

						}
						else
						{
							ct.Angle = Math.PI;
							mRadius = paax.Pos.Distance(pbax.Pos) / 2.0;
						}
						Direction radiusDir = ori.AbsoluteDirection(pbax.Dir.Orthogonal(normal));
						ct.Center = new Point(ct.StartPnt).MoveBy(radiusDir, mRadius);

						session.Save(ct);
					}
					else if (gEle.GetElementType() == DbElementTypeInstance.LSNOUT)
					{
						string expr = gEle.GetAsString(DbAttributeInstance.PAAX);
						AxisDir paax = EvalDirection.Eval(ele, expr);

						expr = gEle.GetAsString(DbAttributeInstance.PBAX);
						AxisDir pbax = EvalDirection.Eval(ele, expr);

						double ptdi = GetExper(gEle, DbAttributeInstance.PTDI).Eval(ele);
						double pbdi = GetExper(gEle, DbAttributeInstance.PBDI).Eval(ele);
						double ptdm = GetExper(gEle, DbAttributeInstance.PTDM).Eval(ele);
						double pbdm = GetExper(gEle, DbAttributeInstance.PBDM).Eval(ele);
						double poff = GetExper(gEle, DbAttributeInstance.POFF).Eval(ele);

						Snout snout = new Snout();
						snout.ButtomRadius = pbdm / 2.0;
						snout.TopRadius = ptdm / 2.0;

						Aveva.Pdms.Geometry.Orientation ori = ele.GetOrientation(DbAttributeInstance.ORI);
						Direction tdir = ori.AbsoluteDirection(paax.Dir);
						Position pos = Position.Create();
						double dist = paax.Pos.Distance(pos);
						if (dist > 0.00001)
						{
							pos.MoveBy(ori.AbsoluteDirection(Direction.Create(paax.Pos)), dist);
						}

						Direction bdir = ori.AbsoluteDirection(pbax.Dir);

						snout.Org = new Point(pos).MoveBy(ele.GetPosition(DbAttributeInstance.POS)).MoveBy(tdir, pbdi);
						snout.Offset = new Point(bdir).Mul(poff);
						snout.Height = new Point(tdir).Mul(ptdi - pbdi);

						session.Save(snout);
					}
					else if (gEle.GetElementType() == DbElementTypeInstance.SDSH)
					{
						string expr = gEle.GetAsString(DbAttributeInstance.PAXI);
						AxisDir paxi = EvalDirection.Eval(ele, expr);

						double phei = GetExper(gEle, DbAttributeInstance.PHEI).Eval(ele);
						double pdia = GetExper(gEle, DbAttributeInstance.PDIA).Eval(ele);
						double pdis = GetExper(gEle, DbAttributeInstance.PDIS).Eval(ele);
						double prad = GetExper(gEle, DbAttributeInstance.PRAD).Eval(ele);

						Aveva.Pdms.Geometry.Orientation ori = ele.GetOrientation(DbAttributeInstance.ORI);
						Direction dir = ori.AbsoluteDirection(paxi.Dir);
						Position pos = Position.Create();
						double dist = paxi.Pos.Distance(pos);
						if (dist > 0.00001)
						{
							pos.MoveBy(ori.AbsoluteDirection(Direction.Create(paxi.Pos)), dist);
						}

						Dish dish = new Dish();
						dish.Org = new Point(pos)
							.MoveBy(ele.GetPosition(DbAttributeInstance.POS))
							.MoveBy(dir, pdis);
						dish.Height = new Point(dir).Mul(phei);
						dish.Radius = pdia / 2.0;
						dish.IsEllipse = prad > 0.0;
						session.Save(dish);
					}
				}
				gEle = gEle.Next();
			}
		}

		private void ExportEquip(DbElement equipEle)
		{
			DbElement ele = equipEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				if (IsReadableEle(ele))
				{
					if (ele.IsAttributeSetable(DbAttributeInstance.LEVE) && ele.IsAttributeValid(DbAttributeInstance.LEVE))
					{
						int[] level = ele.GetIntegerArray(DbAttributeInstance.LEVE);
						if (level.Length >= 2 && level[1] < 6)
						{
							ele = ele.Next();
							continue;
						}
					}

					if (ele.GetElementType() == DbElementTypeInstance.NOZZLE)
						ExportPipeItem(ele);
					else if (ele.GetElementType() == DbElementTypeInstance.CYLINDER)
					{
						Aveva.Pdms.Geometry.Orientation ori = ele.GetOrientation(DbAttributeInstance.ORI);
						Cylinder cyl = new Cylinder();
						cyl.Height = new Point(ori.AbsoluteDirection(Direction.Create(Axis.UP)))
							.Mul(ele.GetDouble(DbAttributeInstance.HEIG));
						cyl.Org = new Point(ele.GetPosition(DbAttributeInstance.POS)).MoveBy(cyl.Height, -0.5);
						cyl.Radius = ele.GetDouble(DbAttributeInstance.DIAM) / 2.0;
						session.Save(cyl);
					}
					else if (ele.GetElementType() == DbElementTypeInstance.BOX)
					{
						Aveva.Pdms.Geometry.Orientation ori = ele.GetOrientation(DbAttributeInstance.ORI);
						Position pos = ele.GetPosition(DbAttributeInstance.POS);
						double xlen = ele.GetDouble(DbAttributeInstance.XLEN);
						double ylen = ele.GetDouble(DbAttributeInstance.YLEN);
						double zlen = ele.GetDouble(DbAttributeInstance.ZLEN);

						Box box = new Box();
						box.XLen = new Point(ori.AbsoluteDirection(Direction.Create(Axis.EAST))).Mul(xlen);
						box.YLen = new Point(ori.AbsoluteDirection(Direction.Create(Axis.NORTH))).Mul(ylen);
						box.ZLen = new Point(ori.AbsoluteDirection(Direction.Create(Axis.UP))).Mul(zlen);
						box.Org = new Point(pos).MoveBy(box.XLen, -0.5).MoveBy(box.YLen, -0.5).MoveBy(box.ZLen, -0.5);
						session.Save(box);
					}
					else if (ele.GetElementType() == DbElementTypeInstance.DISH)
					{
						Aveva.Pdms.Geometry.Orientation ori = ele.GetOrientation(DbAttributeInstance.ORI);
						Position pos = ele.GetPosition(DbAttributeInstance.POS);

						Dish dish = new Dish();
						dish.Org = new Point(pos);
						dish.Height = new Point(ori.AbsoluteDirection(Direction.Create(Axis.EAST)))
							.Mul(ele.GetDouble(DbAttributeInstance.HEIG));
						dish.Radius = ele.GetDouble(DbAttributeInstance.DIAM) / 2.0;
						dish.IsEllipse = ele.GetDouble(DbAttributeInstance.RADI) > 0.0;
						session.Save(dish);
					}
					else if (ele.GetElementType() == DbElementTypeInstance.CONE)
					{
						Aveva.Pdms.Geometry.Orientation ori = ele.GetOrientation(DbAttributeInstance.ORI);
						Position pos = ele.GetPosition(DbAttributeInstance.POS);
						Direction dir = ori.AbsoluteDirection(Direction.Create(Axis.UP));
						double height = ele.GetDouble(DbAttributeInstance.HEIG);

						Cone cone = new Cone();
						cone.Org = new Point(pos).MoveBy(dir, -height / 2.0);
						cone.Height = new Point(dir)
							.Mul(height);
						cone.TopRadius = ele.GetDouble(DbAttributeInstance.DTOP) / 2.0;
						cone.ButtomRadius = ele.GetDouble(DbAttributeInstance.DBOT) / 2.0;
						session.Save(cone);
					}
				}
				ele = ele.Next();
			}
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