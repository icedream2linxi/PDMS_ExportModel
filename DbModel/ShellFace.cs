using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NHibernate.Mapping.Attributes;

namespace DbModel
{
	[Class(Table="shell_face")]
	public class ShellFace
	{
		[Id(0, TypeType=typeof(int))]
		[Key(1)]
		[Generator(2, Class="native")]
		public virtual int ID { get; set; }

		[Property(Column = "vertex_index")]
		public virtual int VertexIndex { get; set; }

		[ManyToOne(Column = "shell_id")]
		public virtual Shell Shell { get; set; }
	}
}
