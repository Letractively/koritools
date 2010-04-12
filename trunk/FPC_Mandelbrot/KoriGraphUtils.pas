unit KoriGraphUtils;

interface

type
  TPoint=record
    x: Extended;
    y: Extended;
  end;

  TScreenPoint=record
    x: Integer;
    y: Integer;
  end;


  TPointConverter = class
  private
    // KISS = Keep it simple, stupid
    Cy: Extended;
    Cx: Extended;
  public
    constructor Create(xFrame: Extended; yFrame: Extended);
    procedure Point2ScreenPoint(const P: TPoint; var Ps: TScreenPoint);
    procedure ScreenPoint2Point(const Ps: TScreenPoint; var P: TPoint);
  end;

function InitGraphicMode: Boolean;


implementation

uses
  Graph;

function InitGraphicMode: Boolean;
var
  GraphDriver: SmallInt;
  GraphMode: SmallInt;
begin
  GraphDriver := 0;
  GraphMode := 0;
  DetectGraph(GraphDriver, GraphMode);
  InitGraph(GraphDriver, GraphMode, '');
  InitGraphicMode := (GraphResult = 0);
end;

constructor TPointConverter.Create(xFrame: Extended; yFrame: Extended);
begin
  Assert( (xFrame > 0) and (yFrame > 0) );
  Cy := 2.0*yFrame / GetMaxY;
  Cx := 2.0*xFrame / GetMaxX;
end;

procedure TPointConverter.Point2ScreenPoint(const P: TPoint; var Ps: TScreenPoint);
begin
  Ps.y := Round(GetMaxY/2.0 - P.y/Cy);
  Ps.x := Round(GetMaxX/2.0 + P.x/Cx);
end;

procedure TPointConverter.ScreenPoint2Point(const Ps: TScreenPoint; var P: TPoint);
begin
  P.x := (Ps.x - GetMaxX/2.0)*Cx;
  P.y := (GetMaxY/2.0 - Ps.y)*Cy;
end;

end.
