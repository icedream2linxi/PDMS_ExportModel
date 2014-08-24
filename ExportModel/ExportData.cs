using Aveva.Pdms.Database;
using Aveva.Pdms.Geometry;
using Aveva.Pdms.Maths.Geometry;
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

		private D3Transform GetTransform(DbElement ele)
		{
			return GeometryUtility.ToD3Transform(ele.GetOrientation(DbAttributeInstance.ORI), ele.GetPosition(DbAttributeInstance.POS));
		}

		private int GetColor(DbElement ele, int parentColor)
		{
			DbElementType eleType = ele.GetElementType();
			if (eleType == DbElementTypeInstance.BRANCH)
				return System.Drawing.Color.Gold.ToArgb();
			else if (eleType == DbElementTypeInstance.EQUIPMENT)
				return System.Drawing.Color.Tan.ToArgb();
			else
				return parentColor;
		}

		private int GetColor(DbElement ele)
		{
			List<DbElement> eleArr = new List<DbElement>();
			do
			{
				eleArr.Add(ele);
			}
			while (IsReadableEle(ele = ele.Owner));

			int color = System.Drawing.Color.White.ToArgb();
			for (int i = eleArr.Count-1; i >= 0; --i)
			{
				color = GetColor(eleArr[i], color);
			}
			return color;
		}

		private void Export(DbElement ele)
		{
			if (ele.GetElementType() == DbElementTypeInstance.WORLD)
				ExportWorld(ele);
			else if (ele.GetElementType() == DbElementTypeInstance.SITE)
				ExportSite(ele, GetColor(ele.Owner));
			else if (ele.GetElementType() == DbElementTypeInstance.ZONE)
			{
				D3Transform transform = GetTransform(ele.Owner);
				ExportZone(ele, transform, GetColor(ele.Owner));
			}
			else if (ele.GetElementType() == DbElementTypeInstance.PIPE)
			{
				DbElement zoneEle = ele.Owner;
				DbElement siteEle = zoneEle.Owner;
				D3Transform transform = GetTransform(siteEle).Multiply(GetTransform(zoneEle));
				ExportPipe(ele, transform, GetColor(ele.Owner));
			}
			else if (ele.GetElementType() == DbElementTypeInstance.EQUIPMENT)
			{
				DbElement zoneEle = ele.Owner;
				DbElement siteEle = zoneEle.Owner;
				D3Transform transform = GetTransform(siteEle).Multiply(GetTransform(zoneEle));
				ExportEquip(ele, transform, GetColor(ele.Owner));
			}
			else if (ele.GetElementType() == DbElementTypeInstance.BRANCH)
			{
				DbElement pipeEle = ele.Owner;
				DbElement zoneEle = pipeEle.Owner;
				DbElement siteEle = zoneEle.Owner;
				D3Transform transform = GetTransform(siteEle).Multiply(GetTransform(zoneEle));
				ExportBranch(ele, transform, GetColor(ele.Owner));
			}
			SaveExpr();
		}

		private bool IsReadableEle(DbElement ele)
		{
			return ele.IsValid && !ele.IsNull && !ele.IsDeleted;
		}

		private void ExportWorld(DbElement worldEle)
		{
			int color = GetColor(worldEle, System.Drawing.Color.White.ToArgb());
			DbElement ele = worldEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				if (IsReadableEle(ele) && ele.GetElementType() == DbElementTypeInstance.SITE)
					ExportSite(ele, color);
				ele = ele.Next();
			}
		}

		private void ExportSite(DbElement siteEle, int color)
		{
			int curCol = GetColor(siteEle, color);
			D3Transform transform = GetTransform(siteEle);
			DbElement ele = siteEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				if (IsReadableEle(ele) && ele.GetElementType() == DbElementTypeInstance.ZONE)
					ExportZone(ele, transform, curCol);
				ele = ele.Next();
			}
		}

		private void ExportZone(DbElement zoneEle, D3Transform transform, int color)
		{
			int curCol = GetColor(zoneEle, color);
			D3Transform currentTransform = transform.Multiply(GetTransform(zoneEle));
			DbElement ele = zoneEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				if (IsReadableEle(ele))
				{
					if (ele.GetElementType() == DbElementTypeInstance.PIPE)
						ExportPipe(ele, currentTransform, curCol);
					else if (ele.GetElementType() == DbElementTypeInstance.EQUIPMENT)
						ExportEquip(ele, currentTransform, curCol);
				}
				ele = ele.Next();
			}
		}

		private void ExportPipe(DbElement pipeEle, D3Transform transform, int color)
		{
			int curCol = GetColor(pipeEle, color);
			DbElement ele = pipeEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				if (IsReadableEle(ele) && ele.GetElementType() == DbElementTypeInstance.BRANCH)
					ExportBranch(ele, transform, curCol);
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

		private void ExportTube(DbElement tubeEle, D3Transform transform, int color)
		{
			double ltLength = tubeEle.GetDouble(DbAttributeInstance.ITLE);
			if (ltLength <= 0.001)
				return;

			double lbore = tubeEle.GetDoubleArray(DbAttributeInstance.PARA)[1];
			D3Vector dir = null;

			DbElement prevEle = tubeEle.Previous;
			if (prevEle == null || !prevEle.IsValid)
			{
				DbElement branchEle = tubeEle.Owner;
				dir = transform.Multiply(GeometryUtility.ToD3VectorRef(branchEle.GetDirection(DbAttributeInstance.HDIR)));
			}
			else
			{
				int leave = prevEle.GetInteger(DbAttributeInstance.LEAV);
				AxisDir ptax = EvalDirection.Eval(prevEle, "P" + leave);
				if (ptax == null)
					return;

				D3Transform eleTrans = transform.Multiply(GetTransform(prevEle));
				dir = eleTrans.Multiply(GeometryUtility.ToD3VectorRef(ptax.Dir));
			}

			D3Point pos = transform.Multiply(GeometryUtility.ToD3Point(tubeEle.GetPosition(DbAttributeInstance.ITPS)));
			pos.MoveBy(dir * (-ltLength / 2));

			Cylinder cyl = new Cylinder();
			cyl.Org = new Point(pos);
			cyl.Height = new Point(dir).Mul(ltLength);
			cyl.Radius = lbore / 2.0;
			cyl.Color = color;
			session.Save(cyl);
		}

		private void ExportBranch(DbElement branchEle, D3Transform transform, int color)
		{
			int curCol = GetColor(branchEle, color);
			DbElement ele = branchEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				if (IsReadableEle(ele))
					ExportPipeItem(ele, transform, curCol);
				ele = ele.Next();
			}
		}

		private void ExportPipeItem(DbElement ele, D3Transform transform, int color)
		{
			if (ele.GetElementType() == DbElementTypeInstance.TUBING)
			{
				ExportTube(ele, transform, color);
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

			D3Transform eleTrans = transform.Multiply(GetTransform(ele));

			DbElement gEle = gmEle.FirstMember();
			while (gEle != null && gEle.IsValid)
			{
				if (IsReadableEle(gEle) && gEle.GetBool(DbAttributeInstance.TUFL))
				{
					if (!IsVisible(gEle))
					{
						gEle = gEle.Next();
						continue;
					}

					try
					{
						if (gEle.GetElementType() == DbElementTypeInstance.SCYLINDER)
						{
							string expr = gEle.GetAsString(DbAttributeInstance.PAXI);
							AxisDir paxi = EvalDirection.Eval(ele, expr);
	
							double phei = GetExper(gEle, DbAttributeInstance.PHEI).Eval(ele);
							double pdia = GetExper(gEle, DbAttributeInstance.PDIA).Eval(ele);
							double pdis = GetExper(gEle, DbAttributeInstance.PDIS).Eval(ele);
	
							D3Vector dir = eleTrans.Multiply(GeometryUtility.ToD3VectorRef(paxi.Dir));
							D3Point pos = eleTrans.Multiply(GeometryUtility.ToD3Point(paxi.Pos));
	
							Cylinder cyl = new Cylinder();
							cyl.Org = new Point(pos)
								.MoveBy(dir, pdis);
							cyl.Height = new Point(dir).Mul(phei);
							cyl.Radius = pdia / 2.0;
							cyl.Color = color;
							session.Save(cyl);
						}
						else if (gEle.GetElementType() == DbElementTypeInstance.LCYLINDER)
						{
							string expr = gEle.GetAsString(DbAttributeInstance.PAXI);
							AxisDir paxi = EvalDirection.Eval(ele, expr);
	
							double pdia = GetExper(gEle, DbAttributeInstance.PDIA).Eval(ele);
							double pbdi = GetExper(gEle, DbAttributeInstance.PBDI).Eval(ele);
							double ptdi = GetExper(gEle, DbAttributeInstance.PTDI).Eval(ele);
	
							D3Vector dir = eleTrans.Multiply(GeometryUtility.ToD3VectorRef(paxi.Dir));
							D3Point pos = eleTrans.Multiply(GeometryUtility.ToD3Point(paxi.Pos));
			
							Cylinder cyl = new Cylinder();
							cyl.Org = new Point(pos)
								.MoveBy(dir, pbdi);
							cyl.Height = new Point(dir).Mul(ptdi - pbdi);
							cyl.Radius = pdia / 2.0;
							cyl.Color = color;
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
	
							Point xlen = new Point(eleTrans.Multiply(D3Vector.D3EAST));
							Point ylen = new Point(eleTrans.Multiply(D3Vector.D3NORTH));
							Point zlen = new Point(eleTrans.Multiply(D3Vector.D3UP));
	
							Point pos = new Point(eleTrans.Multiply(GeometryUtility.Org));
							pos.MoveBy(xlen, px - pxlen / 2.0).MoveBy(ylen, py - pylen / 2.0).MoveBy(zlen, pz - pzlen / 2.0);
							xlen.Mul(pxlen);
							ylen.Mul(pylen);
							zlen.Mul(pzlen);
	
							Box box = new Box();
							box.Org = pos;
							box.XLen = xlen;
							box.YLen = ylen;
							box.ZLen = zlen;
							box.Color = color;
							session.Save(box);
						}
						else if (gEle.GetElementType() == DbElementTypeInstance.SCTORUS)
						{
							string expr = gEle.GetAsString(DbAttributeInstance.PAAX);
							AxisDir paax = EvalDirection.Eval(ele, expr);
	
							expr = gEle.GetAsString(DbAttributeInstance.PBAX);
							AxisDir pbax = EvalDirection.Eval(ele, expr);
	
							double pdia = GetExper(gEle, DbAttributeInstance.PDIA).Eval(ele);
	
							CircularTorus ct = new CircularTorus();
							Direction normal = null;
							if (!paax.Dir.IsParallel(pbax.Dir))
								normal = paax.Dir.Orthogonal(pbax.Dir);
							else
								normal = paax.Dir.Orthogonal(Direction.Create(paax.Pos, pbax.Pos));
							ct.Normal = new Point(eleTrans.Multiply(GeometryUtility.ToD3VectorRef(normal)));
	
							ct.StartPnt = new Point(eleTrans.Multiply(GeometryUtility.ToD3Point(pbax.Pos)));
							ct.StartRadius = ct.EndRadius = pdia / 2.0;
	
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
							D3Vector radiusDir = eleTrans.Multiply(GeometryUtility.ToD3VectorRef(pbax.Dir.Orthogonal(normal)));
							ct.Center = new Point(ct.StartPnt).MoveBy(radiusDir, mRadius);
							ct.Color = color;
	
							session.Save(ct);
						}
						else if (gEle.GetElementType() == DbElementTypeInstance.SRTORUS)
						{
							string expr = gEle.GetAsString(DbAttributeInstance.PAAX);
							AxisDir paax = EvalDirection.Eval(ele, expr);
	
							expr = gEle.GetAsString(DbAttributeInstance.PBAX);
							AxisDir pbax = EvalDirection.Eval(ele, expr);
	
							double pdia = GetExper(gEle, DbAttributeInstance.PDIA).Eval(ele);
							double phei = GetExper(gEle, DbAttributeInstance.PHEI).Eval(ele);
	
							RectangularTorus rt = new RectangularTorus();
							Direction normal = null;
							if (!paax.Dir.IsParallel(pbax.Dir))
								normal = paax.Dir.Orthogonal(pbax.Dir);
							else
								normal = paax.Dir.Orthogonal(Direction.Create(paax.Pos, pbax.Pos));
							rt.Normal = new Point(eleTrans.Multiply(GeometryUtility.ToD3VectorRef(normal)));
	
							rt.StartPnt = new Point(eleTrans.Multiply(GeometryUtility.ToD3Point(pbax.Pos)));
							rt.StartWidth = rt.EndWidth = pdia;
							rt.StartHeight = rt.EndHeight = phei;
	
							double mRadius = 0.0;
							if (!paax.Dir.IsParallel(pbax.Dir))
							{
								double ang = paax.Dir.Angle(pbax.Dir) * Math.PI / 180.0;
								rt.Angle = Math.PI - ang;
								ang /= 2.0;
								double len = paax.Pos.Distance(pbax.Pos) / 2;
								mRadius = len / Math.Sin(ang) * Math.Tan(ang);
	
							}
							else
							{
								rt.Angle = Math.PI;
								mRadius = paax.Pos.Distance(pbax.Pos) / 2.0;
							}
							D3Vector radiusDir = eleTrans.Multiply(GeometryUtility.ToD3VectorRef(pbax.Dir.Orthogonal(normal)));
							rt.Center = new Point(rt.StartPnt).MoveBy(radiusDir, mRadius);
							rt.Color = color;
	
							session.Save(rt);
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
							snout.BottomRadius = pbdm / 2.0;
							snout.TopRadius = ptdm / 2.0;
	
							D3Vector tdir = eleTrans.Multiply(GeometryUtility.ToD3VectorRef(paax.Dir));
							D3Point pos = eleTrans.Multiply(GeometryUtility.ToD3Point(paax.Pos));
							D3Vector bdir = eleTrans.Multiply(GeometryUtility.ToD3VectorRef(pbax.Dir));
	
							snout.Org = new Point(pos).MoveBy(tdir, pbdi);
							snout.Offset = new Point(bdir).Mul(poff);
							snout.Height = new Point(tdir).Mul(ptdi - pbdi);
							snout.Color = color;
	
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
	
							D3Vector dir = eleTrans.Multiply(GeometryUtility.ToD3VectorRef(paxi.Dir));
							if (prad > 0.0)
							{
								Ellipsoid ellipsoid = new Ellipsoid();
								ellipsoid.Center = new Point(eleTrans.Multiply(GeometryUtility.ToD3Point(paxi.Pos)))
									.MoveBy(dir, pdis);
								ellipsoid.ALen = new Point(dir).Mul(phei);
								ellipsoid.BRadius = pdia / 2.0;
								ellipsoid.Color = color;
								session.Save(ellipsoid);
							}
							else
							{
								Sphere sphere = new Sphere();
								sphere.Center = new Point(eleTrans.Multiply(GeometryUtility.ToD3Point(paxi.Pos)))
									.MoveBy(dir, pdis);
								double bottomRadius = pdia / 2.0;
								sphere.Radius = (bottomRadius * bottomRadius + phei * phei) / 2.0 / phei;
								sphere.BottomNormal = new Point(-dir);
								double angle = Math.PI / 2.0 - Math.Asin(2.0 * bottomRadius * phei / (bottomRadius * bottomRadius + phei * phei));
								if (bottomRadius >= phei)
									sphere.Angle = Math.PI - angle * 2.0;
								else
									sphere.Angle = Math.PI + angle * 2.0;
								sphere.Color = color;
								session.Save(sphere);
							}
						}
						else if (gEle.GetElementType() == DbElementTypeInstance.LPYRAMID)
						{
							string exper = gEle.GetAsString(DbAttributeInstance.PAAX);
							AxisDir paax = EvalDirection.Eval(ele, exper);
							exper = gEle.GetAsString(DbAttributeInstance.PBAX);
							AxisDir pbax = EvalDirection.Eval(ele, exper);
							exper = gEle.GetAsString(DbAttributeInstance.PCAX);
							AxisDir pcax = EvalDirection.Eval(ele, exper);
	
							double pbtp = GetExper(gEle, DbAttributeInstance.PBTP).Eval(ele);
							double pctp = GetExper(gEle, DbAttributeInstance.PCTP).Eval(ele);
							double pbbt = GetExper(gEle, DbAttributeInstance.PBBT).Eval(ele);
							double pcbt = GetExper(gEle, DbAttributeInstance.PCBT).Eval(ele);
							double pbof = GetExper(gEle, DbAttributeInstance.PBOF).Eval(ele);
							double pcof = GetExper(gEle, DbAttributeInstance.PCOF).Eval(ele);
							double ptdi = GetExper(gEle, DbAttributeInstance.PTDI).Eval(ele);
							double pbdi = GetExper(gEle, DbAttributeInstance.PBDI).Eval(ele);
	
							D3Vector zDir = eleTrans.Multiply(GeometryUtility.ToD3VectorRef(paax.Dir));
							D3Vector xDir = eleTrans.Multiply(GeometryUtility.ToD3VectorRef(pbax.Dir));
							D3Vector yDir = eleTrans.Multiply(GeometryUtility.ToD3VectorRef(pcax.Dir));
							D3Point org = eleTrans.Multiply(GeometryUtility.ToD3Point(paax.Pos));
							org.MoveBy(zDir * pbdi);
	
							double height = ptdi - pbdi;
	
							Pyramid pyramid = new Pyramid();
							pyramid.Org = new Point(org);
							pyramid.Height = new Point(zDir).Mul(height);
							pyramid.XAxis = new Point(xDir);
							pyramid.BottomXLen = pbbt;
							pyramid.BottomYLen = pcbt;
							pyramid.TopXLen = pbtp;
							pyramid.TopYLen = pctp;
							pyramid.Offset = new Point(xDir * pbof + yDir * pcof);
							pyramid.Color = color;
							session.Save(pyramid);
						}
					}
					catch (System.NullReferenceException )
					{
						
					}
					catch (System.Exception)
					{
						System.Console.WriteLine("Ele = " + ele.GetAsString(DbAttributeInstance.NAME));
						System.Console.WriteLine("GEle = " + gEle.GetAsString(DbAttributeInstance.NAME));
						throw;
					}
				}
				gEle = gEle.Next();
			}
		}

		private bool IsVisible(DbElement ele)
		{
			if (ele.IsAttributeValid(DbAttributeInstance.LEVE))
			{
				int[] level = ele.GetIntegerArray(DbAttributeInstance.LEVE);
				if (level.Length >= 2 && level[1] < 6)
				{
					return false;
				}
			}
			return true;
		}

		private void ExportEquip(DbElement equipEle, D3Transform transform, int parentColor)
		{
			int color = GetColor(equipEle, parentColor);
			D3Transform currTrans = transform.Multiply(GetTransform(equipEle));
			DbElement ele = equipEle.FirstMember();
			while (ele != null && ele.IsValid)
			{
				if (IsReadableEle(ele))
				{
					if (!IsVisible(ele))
					{
						ele = ele.Next();
						continue;
					}

					DbElementType eleType = ele.GetElementType();
					if (eleType == DbElementTypeInstance.NOZZLE)
						ExportPipeItem(ele, currTrans, parentColor);
					else if (eleType == DbElementTypeInstance.TMPLATE)
					{
						DbElement[] lcnfArray = null;
						if (ele.IsAttributeValid(DbAttributeInstance.LCNFA))
						{
							lcnfArray = ele.GetElementArray(DbAttributeInstance.LCNFA);
						}

						if (lcnfArray == null || lcnfArray.Length <= 0)
						{
							DbElement tmplEle = ele.FirstMember();
							while (tmplEle != null && tmplEle.IsValid)
							{
								if (IsReadableEle(tmplEle) && IsVisible(tmplEle))
									ExportDesignGeomotry(tmplEle, currTrans, color);
								tmplEle = tmplEle.Next();
							}
						}
						else
						{
							foreach (DbElement lcnfEle in lcnfArray)
							{
								if (IsReadableEle(lcnfEle) && IsVisible(lcnfEle))
									ExportDesignGeomotry(lcnfEle, currTrans, color);
							}
						}
					}
					else if (eleType == DbElementTypeInstance.SUBEQUIPMENT)
					{
						ExportEquip(ele, currTrans, color);
					}
					else
						ExportDesignGeomotry(ele, currTrans, color);
					
				}
				ele = ele.Next();
			}
		}

		private bool IsSupportedDesignGeometryEle(DbElement ele)
		{
			DbElementType type = ele.GetElementType();
			return type == DbElementTypeInstance.CYLINDER
				|| type == DbElementTypeInstance.BOX
				|| type == DbElementTypeInstance.DISH
				|| type == DbElementTypeInstance.CONE
				|| type == DbElementTypeInstance.PYRAMID
				;
		}

		private void ExportDesignGeomotry(DbElement ele, D3Transform currTrans, int color)
		{
			if (ele.GetElementType() == DbElementTypeInstance.CYLINDER)
			{
				D3Transform eleTrans = currTrans.Multiply(GetTransform(ele));
				Cylinder cyl = new Cylinder();
				cyl.Height = new Point(eleTrans.Multiply(D3Vector.D3UP))
					.Mul(ele.GetDouble(DbAttributeInstance.HEIG));
				cyl.Org = new Point(eleTrans.Multiply(GeometryUtility.Org)).MoveBy(cyl.Height, -0.5);
				cyl.Radius = ele.GetDouble(DbAttributeInstance.DIAM) / 2.0;
				cyl.Color = color;
				session.Save(cyl);
			}
			else if (ele.GetElementType() == DbElementTypeInstance.BOX)
			{
				D3Transform eleTrans = currTrans.Multiply(GetTransform(ele));
				double xlen = ele.GetDouble(DbAttributeInstance.XLEN);
				double ylen = ele.GetDouble(DbAttributeInstance.YLEN);
				double zlen = ele.GetDouble(DbAttributeInstance.ZLEN);

				Box box = new Box();
				box.XLen = new Point(eleTrans.Multiply(D3Vector.D3EAST)).Mul(xlen);
				box.YLen = new Point(eleTrans.Multiply(D3Vector.D3NORTH)).Mul(ylen);
				box.ZLen = new Point(eleTrans.Multiply(D3Vector.D3UP)).Mul(zlen);
				box.Org = new Point(eleTrans.Multiply(GeometryUtility.Org))
					.MoveBy(box.XLen, -0.5).MoveBy(box.YLen, -0.5).MoveBy(box.ZLen, -0.5);
				box.Color = color;
				session.Save(box);
			}
			else if (ele.GetElementType() == DbElementTypeInstance.DISH)
			{
				D3Transform eleTrans = currTrans.Multiply(GetTransform(ele));

				D3Vector normal = eleTrans.Multiply(D3Vector.D3UP);
				double h = ele.GetDouble(DbAttributeInstance.HEIG);
				double bottomRadius = ele.GetDouble(DbAttributeInstance.DIAM) / 2.0;
				if (ele.GetDouble(DbAttributeInstance.RADI) > 0.0)
				{
					Ellipsoid ellipsoid = new Ellipsoid();
					ellipsoid.Center = new Point(eleTrans.Multiply(GeometryUtility.Org));
					ellipsoid.ALen = new Point(normal).Mul(ele.GetDouble(DbAttributeInstance.HEIG));
					ellipsoid.BRadius = bottomRadius;
					ellipsoid.Color = color;
					session.Save(ellipsoid);
				}
				else
				{
					Sphere sphere = new Sphere();
					sphere.Center = new Point(eleTrans.Multiply(GeometryUtility.Org));
					sphere.BottomNormal = new Point(-normal);
					sphere.Radius = (bottomRadius * bottomRadius + h * h) / 2.0 / h;
					sphere.Angle = Math.PI / 2.0 - Math.Asin(2.0 * bottomRadius * h / (bottomRadius * bottomRadius + h * h));
					if (bottomRadius >= h)
						sphere.Angle = Math.PI - sphere.Angle * 2.0;
					else
						sphere.Angle = Math.PI + sphere.Angle * 2.0;
					sphere.Color = color;
					session.Save(sphere);
				}
			}
			else if (ele.GetElementType() == DbElementTypeInstance.CONE)
			{
				D3Transform eleTrans = currTrans.Multiply(GetTransform(ele));
				D3Point pos = eleTrans.Multiply(GeometryUtility.Org);
				D3Vector dir = eleTrans.Multiply(D3Vector.D3UP);
				double height = ele.GetDouble(DbAttributeInstance.HEIG);

				Cone cone = new Cone();
				cone.Org = new Point(pos).MoveBy(dir, -height / 2.0);
				cone.Height = new Point(dir)
					.Mul(height);
				cone.TopRadius = ele.GetDouble(DbAttributeInstance.DTOP) / 2.0;
				cone.BottomRadius = ele.GetDouble(DbAttributeInstance.DBOT) / 2.0;
				cone.Color = color;
				session.Save(cone);
			}
			else if (ele.GetElementType() == DbElementTypeInstance.PYRAMID)
			{
				D3Transform eleTrans = currTrans.Multiply(GetTransform(ele));
				D3Point pos = eleTrans.Multiply(GeometryUtility.Org);
				D3Vector zDir = eleTrans.Multiply(D3Vector.D3UP);
				D3Vector xDir = eleTrans.Multiply(D3Vector.D3EAST);
				D3Vector yDir = eleTrans.Multiply(D3Vector.D3NORTH);
				double height = ele.GetDouble(DbAttributeInstance.HEIG);
				double xBottom = ele.GetDouble(DbAttributeInstance.XBOT);
				double yBottom = ele.GetDouble(DbAttributeInstance.YBOT);
				double xTop = ele.GetDouble(DbAttributeInstance.XTOP);
				double yTop = ele.GetDouble(DbAttributeInstance.YTOP);
				double xOffset = ele.GetDouble(DbAttributeInstance.XOFF);
				double yOffset = ele.GetDouble(DbAttributeInstance.YOFF);

				Pyramid pyramid = new Pyramid();
				pyramid.Org = new Point(pos).MoveBy(zDir, -height / 2.0);
				pyramid.Height = new Point(zDir).Mul(height);
				pyramid.XAxis = new Point(xDir);
				pyramid.Offset = new Point().MoveBy(xDir, xOffset).MoveBy(yDir, yOffset);
				pyramid.BottomXLen = xBottom;
				pyramid.BottomYLen = yBottom;
				pyramid.TopXLen = xTop;
				pyramid.TopYLen = yTop;
				pyramid.Color = color;
				session.Save(pyramid);
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
