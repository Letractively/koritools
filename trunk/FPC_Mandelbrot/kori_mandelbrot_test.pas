program MandelbrotSet;

uses Math, Graph, KoriGraphUtils;


procedure DrawMandelbrotSet;
const
  BallOutNum = 2.0;
  Sqr_BallOutNum = 4.0;
  Delta = 0.002;

  function PointInMandelbrotSet(const P: TPoint): Boolean;
  const
    N = 2000;
  var
    x,y: Extended;
    xt,yt: Extended;
    i: Integer;
  begin
    x := P.x;
    y := P.y;
    Result := True;
    for i:= 0 to N do
    begin
      if Sqr(x) + Sqr(y) >= Sqr_BallOutNum then
      begin
        Result := False;
        Break;
      end;
      xt := x;
      yt := y;
      x := Sqr(xt) - Sqr(yt) + P.x;
      y := 2.0*xt*yt + P.y;
    end;
  end;

var
  x0,y0: Extended;
  TestPoint: TPoint;
  ScreenPoint: TScreenPoint;
  C: TPointConverter;
begin
  x0 := -1.0 * BallOutNum;
  C := TPointConverter.Create(BallOutNum, (GetMaxY/GetMaxX)*BallOutNum);
  while x0 < BallOutNum do
  begin
    y0 := -1.0 * BallOutNum;
    while y0 < BallOutNum do
    begin
      TestPoint.x := x0;
      TestPoint.y := y0;
      if PointInMandelbrotSet(TestPoint) then
      begin
        C.Point2ScreenPoint(TestPoint, ScreenPoint);
        PutPixel(ScreenPoint.x, ScreenPoint.y, GetColor);
      end;
      y0 := y0 + Delta;
    end;
    x0 := x0 + Delta;
  end
end;

begin
  if not InitGraphicMode then
  begin
    WriteLn('Unable to initialize graphic mode.');
    Exit;
  end;

  DrawMandelbrotSet;

  ReadLn;
  CloseGraph;
end.
