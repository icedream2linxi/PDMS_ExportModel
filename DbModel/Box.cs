using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NHibernate.Mapping.Attributes;

namespace DbModel
{
	[Class(Table="Box")]
	public class Box
	{
		[Id(0, TypeType=typeof(int))]
		[Key(1)]
		[Generator(2, Class="native")]
		public virtual int ID { get; set; }
		//[Property(0)]
		//[Type(1, NameType = typeof(PointType))]
		//[Param(2, Name = "prefix", Content = "org")]
		//[Column(1, Name = "org_x")]
		//[Column(1, Name = "org_y")]
		//[Column(1, Name = "org_z")]
		[Point(After = typeof(IdAttribute), Name="Org", Prefix = "org")]
		public virtual Point Org { get; set; }

		//[Property(Column="org_x")]
		//public virtual double OrgX
		//{
		//	get { return Org.X; }
		//	set { Org.X = value; }
		//}

		//[Property(Column="org_y")]
		//public virtual double OrgY
		//{
		//	get { return Org.Y; }
		//	set { Org.Y = value; }
		//}

		//[Property(Column="org_z")]
		//public virtual double OrgZ
		//{
		//	get { return Org.Z; }
		//	set { Org.Z = value; }
		//}
	}
}
