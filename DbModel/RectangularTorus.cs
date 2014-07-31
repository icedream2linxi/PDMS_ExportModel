using NHibernate.Mapping.Attributes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DbModel
{
	[Class(Table = "rectangular_torus")]
	public class RectangularTorus
	{
		[Id(0, TypeType = typeof(int))]
		[Key(1)]
		[Generator(2, Class = "native")]
		public virtual int ID { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Center")]
		public virtual Point Center { get; set; }

		[Point(After = typeof(IdAttribute), Name = "StartPnt", Prefix="start_pnt")]
		public virtual Point StartPnt { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Normal")]
		public virtual Point Normal { get; set; }

		[Property(Column = "start_width")]
		public virtual double StartWidth { get; set; }

		[Property(Column = "start_height")]
		public virtual double StartHeight { get; set; }

		[Property(Column = "end_width")]
		public virtual double EndWidth { get; set; }

		[Property(Column = "end_height")]
		public virtual double EndHeight { get; set; }

		[Property(Column = "angle")]
		public virtual double Angle { get; set; }

		[Property(Column = "color")]
		public virtual int Color { get; set; }
	}
}
