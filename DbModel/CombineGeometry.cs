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

		[Set(0, OrderBy = "id asc")]
		[Key(1, Column = "combine_geometry_id")]
		[OneToMany(1, ClassType = typeof(Mesh))]
		public virtual ISet<Mesh> Meshs { get; set; }

		[Set(0, OrderBy = "id asc")]
		[OneToMany(1, ClassType = typeof(Shell))]
		[Key(1, Column = "combine_geometry_id")]
		public virtual ISet<Shell> Shells { get; set; }

		[Set(0, OrderBy = "id asc")]
		[OneToMany(1, ClassType = typeof(Polygon))]
		[Key(1, Column = "combine_geometry_id")]
		public virtual ISet<Polygon> Polygons { get; set; }

		[Property(Column = "color")]
		public virtual int Color { get; set; }
	}
}
