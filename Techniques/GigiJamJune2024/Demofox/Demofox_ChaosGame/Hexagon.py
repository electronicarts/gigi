import Host
import GigiArray

Host.SetVariable("FrameIndex", "0")

# https://en.wikipedia.org/wiki/Chaos_game#Optimal_value_of_r_for_every_regular_polygon
Host.SetVariable("Fraction", "0.667")

# Calculated from https://www.mathopenref.com/coordpolycalc.html
Host.SetVariable("NumVerts", "6")
Host.SetVariable("Point0", "0.750, 0.067")
Host.SetVariable("Point1", "0.250, 0.067")
Host.SetVariable("Point2", "0.000, 0.500")
Host.SetVariable("Point3", "0.250, 0.933")
Host.SetVariable("Point4", "0.750, 0.933")
Host.SetVariable("Point5", "1.000, 0.500")
