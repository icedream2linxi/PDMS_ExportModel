using NHibernate;
using NHibernate.Type;
using NHibernate.UserTypes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DbModel
{
	[Serializable]
	public class PointType : ICompositeUserType, IParameterizedType
	{
		private string Prefix { get; set; }

		public bool Equals(object x, object y)
		{
			if (x == y)
				return true;
			if (x == null || y == null)
				return true;
			Point lhs = (Point)x;
			Point rhs = (Point)y;
			return lhs.Equals(rhs);
		}

		public int GetHashCode(object x)
		{
			return x.GetHashCode();
		}

		public void SetParameterValues(System.Collections.Generic.IDictionary<string, string> parameters)
		{
			Prefix = parameters["prefix"];
			if (Prefix == null)
				throw new ArgumentException("Not have prefix parameter!");
		}

		public object GetPropertyValue(object component, int property)
		{
			Point pnt = (Point)component;
			switch (property)
			{
				case 0:
					return pnt.X;
				case 1:
					return pnt.Y;
				case 2:
					return pnt.Z;
				default:
					return null;
			}
		}

		public void SetPropertyValue(object component, int property, object value)
		{
			Point pnt = (Point)component;
			switch (property)
			{
				case 0:
					pnt.X = (Double)value;
					break;
				case 1:
					pnt.Y = (Double)value;
					break;
				case 2:
					pnt.Z = (Double)value;
					break;
			}
		}

		public object NullSafeGet(System.Data.IDataReader dr, string[] names, NHibernate.Engine.ISessionImplementor session, object owner)
		{
			Double x = (Double)NHibernateUtil.Double.NullSafeGet(dr, names[0], session, owner);
			Double y = (Double)NHibernateUtil.Double.NullSafeGet(dr, names[1], session, owner);
			Double z = (Double)NHibernateUtil.Double.NullSafeGet(dr, names[2], session, owner);
			if (x == null || y == null || z == null)
				return null;
			return new Point(x, y, z);
		}

		public void NullSafeSet(System.Data.IDbCommand cmd, object value, int index, bool[] settable, NHibernate.Engine.ISessionImplementor session)
		{
			Point point = (value == null) ? new Point() : (Point)value;

			if (settable[0])
				NHibernateUtil.Double.NullSafeSet(cmd, point.X, index++, session);
			if (settable[1])
				NHibernateUtil.Double.NullSafeSet(cmd, point.Y, index++, session);
			if (settable[2])
				NHibernateUtil.Double.NullSafeSet(cmd, point.Z, index++, session);
		}

		public object DeepCopy(object value)
		{
			if (value == null)
				return null;
			Point srcPnt = (Point)value;
			Point dstPnt = new Point();
			dstPnt.X = srcPnt.X;
			dstPnt.Y = srcPnt.Y;
			dstPnt.Z = srcPnt.Z;
			return dstPnt;
		}

		public object Disassemble(object value, NHibernate.Engine.ISessionImplementor session)
		{
			return DeepCopy(value);
		}

		public object Assemble(object cached, NHibernate.Engine.ISessionImplementor session, object owner)
		{
			return DeepCopy(cached);
		}

		public object Replace(object original, object target, NHibernate.Engine.ISessionImplementor session, object owner)
		{
			return DeepCopy(original);
		}


		public bool IsMutable
		{
			get { return true; }
		}

		public string[] PropertyNames
		{
			get { return new string[] { Prefix + "_x", Prefix + "_y", Prefix + "_z" }; }
		}

		public NHibernate.Type.IType[] PropertyTypes
		{
			get { return new IType[] { NHibernateUtil.Double, NHibernateUtil.Double, NHibernateUtil.Double }; }
		}

		public Type ReturnedClass
		{
			get { return typeof(Point); }
		}
	}
}
