import QtQuick

Item {
    id: chart
    property var samples: []
    property var visibleCurves: []
    property var curveDefinitions: []
    property var localization: null
    property real zoom: 1.0
    property real panFraction: 0.0
    property real pressX: 0
    property real pressPan: 0
    property real pinchStartZoom: 1.0

    function sampleSpan() {
        return Math.max(2, Math.min(samples.length, Math.round(samples.length / zoom)))
    }

    function sampleStart() {
        return Math.max(0, Math.min(Math.max(0, samples.length - sampleSpan()),
                                    Math.round(panFraction * Math.max(0, samples.length - sampleSpan()))))
    }

    function clampPan(value) {
        return Math.max(0, Math.min(1, value))
    }

    function setZoom(value, anchorX) {
        if (samples.length < 2) return
        var oldSpan = sampleSpan()
        var oldStart = sampleStart()
        var anchorRatio = Math.max(0, Math.min(1, anchorX / Math.max(1, width)))
        zoom = Math.max(1, Math.min(8, value))
        var newSpan = sampleSpan()
        var desiredStart = oldStart + anchorRatio * (oldSpan - newSpan)
        var maxStart = Math.max(0, samples.length - newSpan)
        panFraction = maxStart === 0 ? 0 : clampPan(desiredStart / maxStart)
        chartCanvas.requestPaint()
    }

    function curveValue(sample, key) {
        var raw = sample[key]
        return raw === undefined || raw === null ? NaN : Number(raw)
    }

    onSamplesChanged: chartCanvas.requestPaint()
    onVisibleCurvesChanged: chartCanvas.requestPaint()
    onZoomChanged: chartCanvas.requestPaint()
    onPanFractionChanged: chartCanvas.requestPaint()

    Rectangle {
        anchors.fill: parent
        radius: 10
        color: "#151a1f"
        border.color: "#303a45"
        border.width: 1
    }

    Canvas {
        id: chartCanvas
        anchors.fill: parent
        anchors.margins: 14

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            var left = 48
            var top = 14
            var right = 14
            var bottom = 30
            var plotWidth = Math.max(1, width - left - right)
            var plotHeight = Math.max(1, height - top - bottom)

            ctx.strokeStyle = "#303a45"
            ctx.lineWidth = 1
            for (var grid = 0; grid <= 4; ++grid) {
                var gy = top + plotHeight * grid / 4
                ctx.beginPath()
                ctx.moveTo(left, gy)
                ctx.lineTo(left + plotWidth, gy)
                ctx.stroke()
            }

            ctx.fillStyle = "#72808d"
            ctx.font = "12px sans-serif"
            ctx.fillText("100%", 4, top + 5)
            ctx.fillText("0%", 16, top + plotHeight)

            if (chart.samples.length < 2 || chart.visibleCurves.length === 0) {
                ctx.fillStyle = "#8f9aa7"
                ctx.font = "15px sans-serif"
                ctx.fillText(chart.localization.text("chart.select.wait"), left + 12, top + plotHeight / 2)
                return
            }

            var start = chart.sampleStart()
            var span = chart.sampleSpan()
            for (var curveIndex = 0; curveIndex < chart.visibleCurves.length; ++curveIndex) {
                var key = chart.visibleCurves[curveIndex]
                var definition = null
                for (var definitionIndex = 0; definitionIndex < chart.curveDefinitions.length; ++definitionIndex) {
                    if (chart.curveDefinitions[definitionIndex].key === key) {
                        definition = chart.curveDefinitions[definitionIndex]
                        break
                    }
                }
                if (!definition) continue

                var minimum = Number.POSITIVE_INFINITY
                var maximum = Number.NEGATIVE_INFINITY
                for (var valueIndex = start; valueIndex < start + span; ++valueIndex) {
                    var value = chart.curveValue(chart.samples[valueIndex], key)
                    if (isNaN(value)) continue
                    minimum = Math.min(minimum, value)
                    maximum = Math.max(maximum, value)
                }
                if (!isFinite(minimum) || !isFinite(maximum)) continue
                if (maximum - minimum < 0.000001) {
                    minimum -= 1
                    maximum += 1
                }

                ctx.strokeStyle = definition.color
                ctx.lineWidth = 2.5
                ctx.beginPath()
                var pointCount = 0
                for (var pointIndex = start; pointIndex < start + span; ++pointIndex) {
                    var pointValue = chart.curveValue(chart.samples[pointIndex], key)
                    if (isNaN(pointValue)) continue
                    var x = left + (pointIndex - start) * plotWidth / Math.max(1, span - 1)
                    var y = top + plotHeight - (pointValue - minimum) / (maximum - minimum) * plotHeight
                    if (pointCount === 0) ctx.moveTo(x, y)
                    else ctx.lineTo(x, y)
                    ++pointCount
                }
                if (pointCount > 1) ctx.stroke()
            }

            var firstTime = chart.samples[start].time
            var lastTime = chart.samples[start + span - 1].time
            ctx.fillStyle = "#72808d"
            ctx.font = "12px sans-serif"
            ctx.fillText(new Date(firstTime).toLocaleTimeString(), left, height - 8)
            var lastText = new Date(lastTime).toLocaleTimeString()
            ctx.fillText(lastText, width - right - ctx.measureText(lastText).width, height - 8)
        }
    }

    MouseArea {
        anchors.fill: parent
        property bool moving: false
        onPressed: {
            chart.pressX = mouse.x
            chart.pressPan = chart.panFraction
            moving = true
        }
        onPositionChanged: {
            if (!moving || chart.samples.length < 2) return
            var maxStart = Math.max(1, chart.samples.length - chart.sampleSpan())
            var sampleDelta = (mouse.x - chart.pressX) / Math.max(1, width) * chart.sampleSpan()
            chart.panFraction = chart.clampPan(chart.pressPan - sampleDelta / maxStart)
        }
        onReleased: moving = false
        onCanceled: moving = false
        onWheel: function(wheel) {
            chart.setZoom(chart.zoom * (wheel.angleDelta.y > 0 ? 1.25 : 0.8), wheel.x)
        }
    }

    PinchArea {
        anchors.fill: parent
        pinch.minimumScale: 1.0
        pinch.maximumScale: 8.0
        onPinchStarted: chart.pinchStartZoom = chart.zoom
        onPinchUpdated: chart.setZoom(chart.pinchStartZoom * pinch.scale, pinch.center.x)
    }
}
