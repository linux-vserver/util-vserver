#!/bin/sh
case $1 in
pre-start)
	;;
post-start)
	;;
pre-stop)
	;;
post-stop)
	;;
*)
	echo $0 pre-start
	echo $0 pre-stop
	echo $0 post-start
	echo $0 post-stop
	;;
esac
