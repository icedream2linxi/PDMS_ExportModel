using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NHibernate.Mapping.Attributes;
using Iesi.Collections.Generic;

namespace DbModel
{
	[Class(Table="polygon")]
	public class Polygon
	{
		[Id(0, TypeType=typeof(int))]
		[Key(1)]
		[Generator(2, Class="native")]
		public virtual int ID { get; set; }

		[Set(0, OrderBy = "id asc")]
		[OneToMany(1, ClassType = typeof(PolygonVertex))]
		[Key(1, Column = "polygon_id")]
		public virtual ISet<PolygonVertex> Vertexs { get; set; }

		[ManyToOne(Column = "combine_geometry_id")]
		public virtual CombineGeometry CombineGeometry { get; set; }
	}
}
