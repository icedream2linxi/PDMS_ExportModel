using NHibernate.Mapping.Attributes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DbModel
{
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

		[Property(Column = "width")]
		public virtual double Width { get; set; }

		[Property(Column = "height")]
		public virtual double Height { get; set; }

		[Property(Column = "angle")]
		public virtual double Angle { get; set; }
	}
}
