using NHibernate.Mapping.Attributes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DbModel
{
	[Class(Table = "prism")]
	public class Prism
	{
		[Id(0, TypeType = typeof(int))]
		[Key(1)]
		[Generator(2, Class = "native")]
		public virtual int ID { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Org")]
		public virtual Point Org { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Height")]
		public virtual Point Height { get; set; }

		[Point(After = typeof(IdAttribute), Name = "BottomStartPnt", Prefix = "bottom_start_pnt")]
		public virtual Point BottomStartPnt { get; set; }

		[Property(Column = "edge_num")]
		public virtual int EdgeNum { get; set; }

		[Property(Column = "color")]
		public virtual int Color { get; set; }
	}
}
