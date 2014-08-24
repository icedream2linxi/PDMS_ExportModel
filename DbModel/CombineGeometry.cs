using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NHibernate.Mapping.Attributes;
using Iesi.Collections.Generic;

namespace DbModel
{
	[Class(Table = "combine_geometry")]
	public class CombineGeometry
	{
		[Id(0, TypeType = typeof(int))]
		[Key(1)]
		[Generator(2, Class = "native")]
		public virtual int ID { get; set; }

		[SetRawXml(After = typeof(IdAttribute), Id = "combine_geometry_id", Name = "Meshs", Type = typeof(Mesh))]
		public virtual ISet<Mesh> Meshs { get; set; }

		[SetRawXml(After = typeof(IdAttribute), Id = "combine_geometry_id", Name = "Shells", Type = typeof(Shell))]
		public virtual ISet<Shell> Shells { get; set; }

		[SetRawXml(After = typeof(IdAttribute), Id = "combine_geometry_id", Name = "Polygons", Type = typeof(Polygon))]
		public virtual ISet<Polygon> Polygons { get; set; }

		[Property(Column = "color")]
		public virtual int Color { get; set; }
	}
}
