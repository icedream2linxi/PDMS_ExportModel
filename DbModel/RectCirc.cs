using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NHibernate.Mapping.Attributes;

namespace DbModel
{
	[Class(Table="rect_circ")]
	public class RectCirc
	{
		[Id(0, TypeType=typeof(int))]
		[Key(1)]
		[Generator(2, Class="native")]
		public virtual int ID { get; set; }

		[Point(After = typeof(IdAttribute), Name = "RectCenter", Prefix = "rect_center")]
		public virtual Point RectCenter { get; set; }

		[Point(After = typeof(IdAttribute), Name = "XLen")]
		public virtual Point XLen { get; set; }

		[Property(Column = "ylen")]
		public virtual double YLen { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Height")]
		public virtual Point Height { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Offset")]
		public virtual Point Offset { get; set; }

		[Property(Column = "radius")]
		public virtual double Radius { get; set; }

		[Property(Column = "color")]
		public virtual int Color { get; set; }
	}
}
