import Host
import GigiArray

Host.SetVariable("FrameIndex", "0")

# https://en.wikipedia.org/wiki/Chaos_game#Optimal_value_of_r_for_every_regular_polygon
Host.SetVariable("Fraction", "0.618")

# Calculated from https://www.mathopenref.com/coordpolycalc.html
Host.SetVariable("NumVerts", "5")
Host.SetVariable("Point0", "0.500, 0.000")
Host.SetVariable("Point1", "0.024, 0.345")
Host.SetVariable("Point2", "0.206, 0.905")
Host.SetVariable("Point3", "0.794, 0.905")
Host.SetVariable("Point4", "0.976, 0.345")
