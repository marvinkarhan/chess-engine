import { AfterViewInit, asNativeElements, Component, ElementRef, EventEmitter, HostListener, Input, OnInit, Output, ViewChild } from '@angular/core';
import { fromEvent } from 'rxjs';
import { filter, tap, takeUntil, take, switchMap } from 'rxjs/operators';
import { Piece, Position } from '../interfaces/Piece';

@Component({
  selector: 'piece',
  templateUrl: './piece.component.html',
  styleUrls: ['./piece.component.scss'],
})
export class PieceComponent implements AfterViewInit {
  @Input() pieceProperties!: Piece;
  @Input() position!: Position;
  @Input() boardWidth = 800;
  @Output() onDragStart = new EventEmitter<number[]>();
  @Output() onDragStop = new EventEmitter<void>();

  get dimensions() {
    return this.boardWidth / 8;
  }

  @ViewChild('piece') private _pieceRef!: ElementRef;
  get pieceEl(): HTMLElement {
    return this._pieceRef.nativeElement;
  }

  constructor() {
  }
  
  ngOnInit() {
        //TODO FIXME
    //Correct the y position
    this.position = {x: this.position.x, y: Math.trunc(this.position.y)};
  }

  ngAfterViewInit(): void {
    this.initDragAndDrop()
  }

  initDragAndDrop() {
    this.pieceEl.style.cursor = 'grab';
    fromEvent<MouseEvent>(this.pieceEl, 'mousedown')
      .pipe(
        tap((event) => {
          this.onDragStart.emit(this.pieceProperties.possibleTargetSquares)
          this.centerElFromEventOnCursor(event);
          this.pieceEl.style.cursor = 'grabbing';
          this.pieceEl.style.zIndex = '1000';
          const matrix = this.getMatrix();
          // set current posistions
          let pos = {
            // The transform positions
            left: +matrix[4],
            top: +matrix[5],
            // Get the current mouse position
            x: event.clientX,
            y: event.clientY,
          };
          // listen for mouse movement
          fromEvent<MouseEvent>(document, 'mousemove')
            .pipe(
              // terminate when mouse is let go
              takeUntil(
                fromEvent<MouseEvent>(document, 'mouseup').pipe(take(1))
              ),
              tap((event) => {
                // How far the mouse has been moved
                const dx = event.clientX - pos.x;
                const dy = event.clientY - pos.y;

                // Scroll the element
                this.setTranslate(
                  pos.left + dx,
                  pos.top + dy
                );
              })
            )
            .subscribe();
        }),
        switchMap(() =>
          // wait until mouse is releases
          fromEvent<MouseEvent>(document, 'mouseup').pipe(take(1))
        ),
        tap(() => {
          // TODO: check if move as legal
          this.resetStyle()
          if(!this.checkBounds()) {
            this.setTranslate(this.position.x * 100, this.position.y * 100, '%');
          } else {
            this.updatePiece();
          }
          this.onDragStop.emit();
        })
      )
      .subscribe();
  }

  private updatePiece() {
    let matrix = this.getMatrix();
      this.position.y = +matrix[5] / 100;
      this.position.x = +matrix[4] / 100;


  }

  private resetStyle() {
    this.pieceEl.style.cursor = 'grab';
    this.pieceEl.style.zIndex = '';
    this.centerOnCell();

  }

  private checkBounds() {
    const matrix = this.getMatrix();
    let [newX,newY] = [+matrix[4] / 100, +matrix[5] / 100];
    let found = this.pieceProperties.possibleTargetSquares.find(
      (positionNumber: number) => {
        const newPositionNumber = newX + newY * 8;
        if(positionNumber === newPositionNumber)
          return positionNumber;
        else
          return undefined
      }
    )
    return !!found;
  }

  private centerOnCell() {
    const matrix = this.getMatrix();
    this.setTranslate(
      (((+matrix[4] + this.dimensions / 2) / this.dimensions) | 0) * this.dimensions,
      (((+matrix[5] + this.dimensions / 2) / this.dimensions) | 0) * this.dimensions
    );
  }

  private centerElFromEventOnCursor(event: MouseEvent) {
    let xDiff = event.offsetX - this.dimensions / 2;
    let yDiff = event.offsetY - this.dimensions / 2;
    const matrix = this.getMatrix();
    this.setTranslate(+matrix[4] + xDiff, +matrix[5] + yDiff);
  }

  private getMatrix() {
    const style = window.getComputedStyle(this.pieceEl);
    return (style.transform.match(/matrix.*\((.+)\)/) || [])[1]?.split(', ');
  }

  private setTranslate(x: number, y: number, unit: 'px'|'%' = 'px') {
    this.pieceEl.style.transform = `translate(${x}${unit}, ${y}${unit})`;
  }
}
