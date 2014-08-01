using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NHibernate.Mapping.Attributes;

namespace DbModel
{
	[Class(Table="saddle")]
	public class Saddle
	{
		[Id(0, TypeType=typeof(int))]
		[Key(1)]
		[Generator(2, Class="native")]
		public virtual int ID { get; set; }

		[Point(After = typeof(IdAttribute), Name="Org")]
		public virtual Point Org { get; set; }

		[Point(After = typeof(IdAttribute), Name = "XLen")]
		public virtual Point XLen { get; set; }

		[Property(Column = "ylen")]
		public virtual double YLen { get; set; }

		[Point(After = typeof(IdAttribute), Name = "ZLen")]
		public virtual Point ZLen { get; set; }

		[Property(Column = "radius")]
		public virtual double radius { get; set; }

		[Property(Column = "color")]
		public virtual int Color { get; set; }
	}
}
