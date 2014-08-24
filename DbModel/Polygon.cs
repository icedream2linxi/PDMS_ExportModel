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

		[SetRawXml(After = typeof(IdAttribute), Id = "polygon_id", Name = "Vertexs", Type = typeof(PolygonVertex))]
		public virtual ISet<PolygonVertex> Vertexs { get; set; }

		[ManyToOne(Column = "combine_geometry_id")]
		public virtual CombineGeometry CombineGeometry { get; set; }
	}
}
